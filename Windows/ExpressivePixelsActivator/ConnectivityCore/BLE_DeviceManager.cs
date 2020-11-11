using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Storage.Streams;
#if WINDOWS_UWP
using Windows.Foundation;
using Windows.UI.Xaml;
#else
using System.Windows;
using System.Windows.Threading;
#endif

namespace Microsoft.Research.EmbeddedDeviceConnectivity
{
    public class ScannedDevice
    {
        public ulong BLEAddress { get; set; }
        public string Name { get; set; }
        public DateTime ScanTime { get; set; }
        public bool ScanResponsePending { get; set; }
        public BLE_Device Device { get; set; }
        public byte[] LastManufacturerData { get; set; }
    }
    


    public class BLE_DeviceManager
    {
        private static UInt16 GAP_APPEARANCE_ID = 1990;
        private static int GAP_ADTYPE_APPEARANCE = 0x19;
        private static int SCANEXPIRY_TIMER_SECONDS = 30;

        public bool _discovering = false;
        private ObservableCollection<BLE_Device> _discoveredDevices = new ObservableCollection<BLE_Device>();
        private Dictionary<ulong, ScannedDevice> _scannedDevices = new Dictionary<ulong, ScannedDevice>();
        private DispatcherTimer _expiryTimer = new DispatcherTimer();
        private BluetoothLEAdvertisementWatcher _bleAdvertisementWatcher = null;
        public event EventHandler<string> DiagnosticsHandler = null;



        public ObservableCollection<BLE_Device> DiscoveredDevices
        {
            get
            {
                return _discoveredDevices;
            }
        }



        public BLE_DeviceManager()
        {
            _expiryTimer.Tick += ExpiryTimer_Tick;
            _expiryTimer.Interval = TimeSpan.FromSeconds(10);
        }



#if WINDOWS_UWP
        private void ExpiryTimer_Tick(object sender, object e)
#else
        private void ExpiryTimer_Tick(object sender, EventArgs e)
#endif
        {
            while (true)
            {
                try
                {
                    var scannedDevice = _scannedDevices.Values.FirstOrDefault(i => (DateTime.Now - i.ScanTime).TotalSeconds > SCANEXPIRY_TIMER_SECONDS);
                    if (scannedDevice != null)
                    {
                        var discoveredItem = _discoveredDevices.FirstOrDefault(i => i.DeviceID == scannedDevice.BLEAddress.ToString());
                        if (discoveredItem != null)
                        {
                            if (discoveredItem.Connected)
                                scannedDevice.ScanTime = DateTime.Now;
                            else
                            {
                                _discoveredDevices.Remove(discoveredItem);
                                _scannedDevices.Remove(scannedDevice.BLEAddress);
                            }
                        }
                        else
                            _scannedDevices.Remove(scannedDevice.BLEAddress);
                    }
                    else
                        break;
                }
                catch(Exception ex)
                {
                    return;
                }
            }
        }



        public void Start()
        {
            if (!_discovering)
            {
                try
                {
                    if (_bleAdvertisementWatcher == null)
                    {
                        _bleAdvertisementWatcher = new BluetoothLEAdvertisementWatcher();
                        _bleAdvertisementWatcher.Received += OnBLEAdvertismentReceived;
                        _bleAdvertisementWatcher.Stopped += OnBLEAdvertismentStopped;
                        _bleAdvertisementWatcher.ScanningMode = BluetoothLEScanningMode.Active;
                    }
                    _bleAdvertisementWatcher.Start();
                    _expiryTimer.Start();
                    _discovering = true;
                    Debug.WriteLine("BLE_DeviceManager DISCOVERY RUNNING");
                }
                catch(Exception ex)
                {
                    Debug.WriteLine("BLE_DeviceManager::Start EXCEPTION " + ex.ToString());
                }
            }
        }



        public void Stop()
        {
            if (_discovering)
            {
                try
                {
                    if (_bleAdvertisementWatcher != null)
                        _bleAdvertisementWatcher.Stop();
                }
                catch { }
                _expiryTimer.Stop();
                _discovering = false;
                Debug.WriteLine("BLE_DeviceManager DISCOVERY PAUSED");
            }
        }



        private void OnBLEAdvertismentStopped(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementWatcherStoppedEventArgs args)
        {
            Debug.WriteLine("#### OnBLEAdvertismentStopped ####");
            DiagnosticsHandler?.Invoke(this, "#### OnBLEAdvertismentStopped ####");
        }



        public async void OnBLEAdvertismentReceived(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // The local name of the advertising device contained within the payload, if any
            string localName = eventArgs.Advertisement.LocalName.TrimEnd('\0');
            UInt16 extractedAppearanceID = 0;

            //Debug.WriteLine("## BLE SCAN ENTRY " + localName);
            try
            {
                // The BLE scan response contains the service IDs 
                if(eventArgs.AdvertisementType == BluetoothLEAdvertisementType.ScanResponse)
                {
                    ScannedDevice scannedDevice = null;

                    // See if the Advertisement has been received for this device
                    if (_scannedDevices.TryGetValue(eventArgs.BluetoothAddress, out scannedDevice))
                    {
                        if (scannedDevice.ScanResponsePending)
                        {
                            // Look for the UART service
                            Guid guid = eventArgs.Advertisement.ServiceUuids.FirstOrDefault(i => i == GattServiceUuids.UART);
                            if (guid != null)
                            {
                                // Device is now registered
                                _scannedDevices[eventArgs.BluetoothAddress].ScanResponsePending = false;

                                BLE_Device bleConntectibleDevice = new BLE_Device(scannedDevice.Name, scannedDevice.BLEAddress);
#if WINDOWS_UWP
                            await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => 
#else
                                Application.Current.Dispatcher.Invoke(() =>
#endif
                                {
                                    _discoveredDevices.Add(bleConntectibleDevice);
                                });
                            }
                        }
                        else
                        {
                           // Debug.WriteLine(eventArgs.Advertisement.ManufacturerData.Count);
                            if (eventArgs.Advertisement.ManufacturerData.Count > 0)
                            {
                                // Only print the first one of the list
                                var manufacturerData = eventArgs.Advertisement.ManufacturerData[0];
                                var data = new byte[manufacturerData.Data.Length];
                                using (var reader = DataReader.FromBuffer(manufacturerData.Data))
                                {
                                    reader.ReadBytes(data);
                                    if (scannedDevice.LastManufacturerData == null || !data.SequenceEqual(scannedDevice.LastManufacturerData))
                                        scannedDevice.LastManufacturerData = data;
                                }
                            }
                        }
                    }
                }

                // Extract the Appearance ID from the advertisement
                BluetoothLEAdvertisementDataSection dataItem = eventArgs.Advertisement.DataSections.FirstOrDefault(i => i.DataType == GAP_ADTYPE_APPEARANCE);
                if(dataItem != null && dataItem.Data.Capacity == sizeof(UInt16))
                {
                    var data = new byte[dataItem.Data.Length];
                    using (var reader = DataReader.FromBuffer(dataItem.Data))
                        reader.ReadBytes(data);
                    extractedAppearanceID = BitConverter.ToUInt16(data, 0);

                    if (extractedAppearanceID == GAP_APPEARANCE_ID)
                    {
                        ScannedDevice scannedDevice = null;

                       // Debug.WriteLine("FOUND CANDIDATE DEVICE " + localName);
                        if (_scannedDevices.TryGetValue(eventArgs.BluetoothAddress, out scannedDevice))
                        {
                            // Update the scan time
                            scannedDevice.ScanTime = DateTime.Now;

                            // See if the device's name has changed
                            if (scannedDevice.Name.CompareTo(localName) != 0)
                            {
                                scannedDevice.Name = localName;                                
#if WINDOWS_UWP
                                await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => 
#else
                                Application.Current.Dispatcher.Invoke(() =>
#endif
                                {
                                    var discoveredItem = _discoveredDevices.FirstOrDefault(i => i.DeviceID == scannedDevice.BLEAddress.ToString());
                                    if (discoveredItem != null)
                                        discoveredItem.DeviceName = localName;
                                });
                            }
                        }
                        else
                        {
                            scannedDevice = new ScannedDevice();
                            scannedDevice.Name = localName;
                            scannedDevice.BLEAddress = eventArgs.BluetoothAddress;
                            scannedDevice.ScanTime = DateTime.Now;
                            scannedDevice.ScanResponsePending = true;
                            _scannedDevices.Add(eventArgs.BluetoothAddress, scannedDevice);
                        }
                    }
                }

                ulong address = eventArgs.BluetoothAddress;
                short rssi = eventArgs.RawSignalStrengthInDBm;
            }
            catch(Exception ex)
            {
                Debug.WriteLine("## BLE SCAN EXIT EXCEPTION " + localName + " " + ex.ToString());
                DiagnosticsHandler?.Invoke(this, "## BLE SCAN EXIT EXCEPTION " + localName + " " + ex.ToString());
            }
            
        }

    }
}
