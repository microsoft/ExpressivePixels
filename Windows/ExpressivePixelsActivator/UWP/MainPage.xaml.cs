using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Microsoft.Research.EmbeddedDeviceConnectivity;
using System.Collections.ObjectModel;
using System.Windows.Input;
using System.ComponentModel;
using Windows.UI.Xaml.Media.Animation;
using System.Threading.Tasks;
using System.Diagnostics;
using System;
using Newtonsoft.Json.Linq;
using Windows.UI.ViewManagement;
using Windows.Foundation;

namespace ExpressivePixelsActivatorUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page, INotifyPropertyChanged
    {
        private BLE_Device _pleaseSelectdevice = null;
        private BLE_Device _bleDevice = null;
        private BLE_DeviceManager _bleDeviceManager = new BLE_DeviceManager();
        private ObservableCollection<string> _storedAnimations = new ObservableCollection<string>();


        #region NotifyProperty
        public event PropertyChangedEventHandler PropertyChanged;

        public void NotifyPropertyChanged(params string[] propertyNames)
        {
            foreach (var propertyName in propertyNames)
                NotifyPropertyChanged(propertyName);
        }

        protected void NotifyPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
        #endregion



        public ICommand PlayAnimationCmd { get; set; }

        public bool IsConnected
        {
            get
            {
                return _bleDevice != null;
            }
        }



        private bool _connecting = false;
        public bool Connecting
        {
            get
            {
                return _connecting;
            }
            set
            {
                _connecting = value;
                NotifyPropertyChanged("Connecting", "IsConnectButtonEnabled");
            }
        }



        public ObservableCollection<string> StoredAnimations
        {
            get
            {
                return _storedAnimations;
            }
        }



        public BLE_DeviceManager DeviceManager
        {
            get
            {
                return _bleDeviceManager;
            }
        }



        public bool IsConnectButtonEnabled
        {
            get
            {
                if (DevicesDropdown.SelectedItem != null)
                {
                    BLE_Device device = DevicesDropdown.SelectedItem as BLE_Device;
                    return (device != null && device != _pleaseSelectdevice && !Connecting);
                }
                return false;
            }
        }



        public MainPage()
        {
            this.InitializeComponent();

            ApplicationView.PreferredLaunchViewSize = new Size(100, 100);
            ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.PreferredLaunchViewSize;

            DataContext = this;
        }



        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            // Start BLE device enumeration
            _bleDeviceManager.Start();

            _pleaseSelectdevice = new BLE_Device("(Select a device)", 0);
            _bleDeviceManager.DiscoveredDevices.Add(_pleaseSelectdevice);
            DevicesDropdown.SelectedValue = _pleaseSelectdevice;
        }



        private void DevicesDropdown_SelectionChanged(object sender, RoutedEventArgs e)
        {
            BLE_Device device = DevicesDropdown.SelectedItem as BLE_Device;

            if (_pleaseSelectdevice != null && device != _pleaseSelectdevice)
                _bleDeviceManager.DiscoveredDevices.Remove(_pleaseSelectdevice);
            NotifyPropertyChanged("IsConnectButtonEnabled");
        }



        private async void Connection_Click(object sender, RoutedEventArgs e)
        {
            // If currently connected then disconnected
            if (_bleDevice != null)
                await Disconnect();
            else
            {
                _bleDevice = DevicesDropdown.SelectedItem as BLE_Device;

                // Try and connect
                Connecting = true;
                if (await _bleDevice.Connect())
                {
                    NotifyPropertyChanged("IsConnected");
                    _bleDevice.ResponseStringEventOccurred += Device_ResponseStringEventOccurred;

                    // Set initial brightness
                    await _bleDevice.SendChannelString("{\"BRIGHTNESS\", 15}");

                    // Retrieve animations stored on the device
                    await _bleDevice.SendChannelString("{\"CMD\", \"ENUMERATE\"}");
                }
                else
                {
                    // A coonection error occurred so flash an appropriate message to the user
                    Connecting = false;
                    Storyboard sbFlashConnectionError = this.Resources["FlashConnectionError"] as Storyboard;
                    sbFlashConnectionError.Begin();
                }
                Connecting = false;
            }
        }



        private async Task Disconnect(bool removeFromList = false)
        {
            // Disconnect device
            if (_bleDevice != null)
            {
                await _bleDevice.Disconnect();
                _bleDevice = null;
                _storedAnimations.Clear();
            }
            NotifyPropertyChanged("IsConnected");
        }



        private void Device_ResponseStringEventOccurred(object sender, string e)
        {
            try
            {
                JObject jsonRoot = JObject.Parse(e);
                JToken tok = null;

                if (jsonRoot.TryGetValue("Name", out tok))
                    _storedAnimations.Add(tok.ToString());
            }
            catch (Exception ex) { }
            Debug.WriteLine(e);
        }



        private async void AnimationPlay_Click(object sender, RoutedEventArgs e)
        {
            var button = (Button)sender;
            string animationName = button.DataContext as string;
            await _bleDevice.SendChannelString("{\"PLAY\", \"" + animationName + "\"}");
        }
    }
}
