using System;
using System.Diagnostics;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
#if !WINDOWS_UWP
using System.Windows;
using System.Windows.Threading;
#endif

namespace Microsoft.Research.EmbeddedDeviceConnectivity
{
    public static class GattServiceUuids
    {
        public static Guid UART = new Guid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    }



    public static class GattServiceNames
    {
        public const string UART = "MPB_TXRX";
    }



    public static class GattCharacteristicsUuid
    {
        public static Guid TX = new Guid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
        public static Guid RX = new Guid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
        public static Guid ATX = new Guid("6E400004-B5A3-F393-E0A9-E50E24DCCA9E");
        public static Guid ARX = new Guid("6E400005-B5A3-F393-E0A9-E50E24DCCA9E");
    }



    public class BLE_Device
    {
        private UARTService             _uartService = null;
        private GattDeviceService       _gattDeviceService = null;
        private BluetoothLEDevice       _bleDevice = null;
        public event EventHandler       Disconnected = null;
        public event EventHandler<string> ResponseStringEventOccurred = null;
        private StringBuilder           _sbResponse = new StringBuilder();

        public string DeviceName { get; set; }
        public string DeviceID { get; set; }
        public ulong BLEMacAddress { get; set; }



        public BLE_Device(string deviceName, ulong bleMacAddress)
        {
            DeviceName = deviceName;
            BLEMacAddress = bleMacAddress;
        }



        public bool IsConnected()
        {
            return _bleDevice != null;
        }



        public bool Connected
        {
            get
            {
                return _bleDevice != null;
            }
        }



        public async Task<bool> Connect()
        {
            try
            {
                _bleDevice = await BluetoothLEDevice.FromBluetoothAddressAsync(BLEMacAddress);
                if (_bleDevice != null)
                {
                    GattDeviceServicesResult gattServicesResult = await _bleDevice.GetGattServicesAsync();
                    foreach (GattDeviceService service in gattServicesResult.Services)
                    {
                        if (service.Uuid.CompareTo(GattServiceUuids.UART) == 0)
                        {
                            _bleDevice.ConnectionStatusChanged += ConnectionStatusChanged;
                            _gattDeviceService = service;
                            _uartService = new UARTService();
                            _uartService.RXReceived += OnUARTServiceRXReceived;
                            bool result = await _uartService.Start(_gattDeviceService, true);
                            if (result)
                                return true;
                            else
                                Debug.WriteLine("BLE_Device Connect UART Service start failure");
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine("BLE_Device::Connect EXCEPTION " + ex.ToString());
            }
            await Disconnect();
            return false;
        }


#if WINDOWS_UWP
        private async void ConnectionStatusChanged(BluetoothLEDevice sender, object args)
#else
        private void ConnectionStatusChanged(BluetoothLEDevice sender, object args)
#endif
        {
            if (sender.ConnectionStatus == BluetoothConnectionStatus.Disconnected && IsConnected())
            {
#if WINDOWS_UWP
                await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {
                    Disconnected?.Invoke(this, EventArgs.Empty);
                });
#else
                Application.Current.Dispatcher.BeginInvoke((Action)(() =>
                {
                    Disconnected?.Invoke(this, EventArgs.Empty);
                    Debug.WriteLine("BLE_Device::ConnectionStatusChanged " + this.DeviceName + " = " + sender.ConnectionStatus.ToString());
                }));
#endif
            }
        }


        private void OnUARTServiceRXReceived(byte[] payload)
        {
            ProcessFrameString(payload);
        }



        public void ProcessFrameString(byte[] payload)
        {
            for (int i = 0; i < payload.Length; i++)
            {
                if (payload[i] == 0x00)
                {
                    AsyncPostStringResponse(_sbResponse.ToString());
                    _sbResponse.Clear();
                }
                else
                    _sbResponse.Append(System.Text.Encoding.ASCII.GetString(new[] { payload[i] }));
            }
        }



        public async Task Disconnect()
        {
            try
            {
                if (_uartService != null)
                    await _uartService.Stop();
                if (_gattDeviceService != null)
                    _gattDeviceService.Dispose();
                if (_bleDevice != null)
                    _bleDevice.Dispose();
            }
            catch(Exception ex)
            {
                Debug.WriteLine("BLE_Device::Disconnect EXCEPTION " + ex.ToString());
            }
            finally
            {
                _uartService = null;
                _gattDeviceService = null;
                _bleDevice = null;
                _sbResponse.Clear();
            }
        }




#if WINDOWS_UWP
        private async Task AsyncPostStringResponse(string response)
#else
        private void AsyncPostStringResponse(string response)
#endif
        {
#if WINDOWS_UWP
            await Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                ResponseStringEventOccurred?.Invoke(this, response);
            });
#else
            Application.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                ResponseStringEventOccurred?.Invoke(this, response);
            }), DispatcherPriority.ContextIdle);
#endif
        }



        public async Task SendChannelString(string cmd)
        {
            byte[] bytesCmd = Encoding.ASCII.GetBytes(cmd);
            byte[] bytesCmdTerminated = new byte[bytesCmd.Length + 1];
            Buffer.BlockCopy(bytesCmd, 0, bytesCmdTerminated, 0, bytesCmd.Length);
            await SendPayload(bytesCmdTerminated, false, true);
        }



        public async Task<bool> SendPayload(byte[] payload, bool allowProgress = false, bool altChannel = false)
        {
            try
            {
                if(_uartService != null)
                    return await _uartService.SendBuffer(payload, altChannel);
            }
            catch
            {

            }
            return false;
        }



        public static string GetBLEMacAddress(ulong address)
        {
            if (address == ulong.MinValue)
                return string.Empty;

            var macadres = address.ToString("x012");
            var regex = "(.{2})(.{2})(.{2})(.{2})(.{2})(.{2})";
            var replace = "$1:$2:$3:$4:$5:$6";
            var newformat = Regex.Replace(macadres, regex, replace);
            return newformat.ToString().ToUpper();
        }
    }
}
