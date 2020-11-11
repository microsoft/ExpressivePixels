using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using Newtonsoft.Json.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Animation;
using System.Windows.Threading;
using Microsoft.Research.EmbeddedDeviceConnectivity;

namespace Microsoft.Research.ExpressivePixelsActivatorWPF
{
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private BLE_Device _bleDevice = null;
        private BLE_Device _pleaseSelectdevice = null;
        private BLE_DeviceManager _bleDeviceManager = new BLE_DeviceManager();
        private DispatcherTimer _brightnessTimer = new DispatcherTimer();
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


        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;

            _brightnessTimer.Tick += BrightnessTimer_Tick;
            _brightnessTimer.Interval = TimeSpan.FromSeconds(1);

            PlayAnimationCmd = new DelegateCommand(OnPlayAnimation);
        }


        #region Properties
        public ICommand PlayAnimationCmd { get; set; }



        public static readonly DependencyProperty IsApplicationActiveProperty = DependencyProperty.Register("IsApplicationActive", typeof(bool), typeof(MainWindow), new PropertyMetadata(false, (d, e) => ((MainWindow)d).NotifyPropertyChanged("IsApplicationActive")));
        public bool IsApplicationActive
        {
            get { return (bool)GetValue(IsApplicationActiveProperty); }
            set { SetValue(IsApplicationActiveProperty, value); }
        }



        private int _currentBrightness = 0;
        public int CurrentBrightness
        {
            get
            {
                return _currentBrightness;
            }
            set
            {
                _currentBrightness = value;
                if (!_brightnessTimer.IsEnabled)
                    _brightnessTimer.Start();
                NotifyPropertyChanged("CurrentBrightness");
            }
        }



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



        public BLE_DeviceManager DeviceManager
        {
            get
            {
                return _bleDeviceManager;
            }
        }



        public ObservableCollection<string> StoredAnimations
        {
            get
            {
                return _storedAnimations;
            }
        }



        public bool IsConnectButtonEnabled
        {
            get
            {
                if (DevicesDropdown.SelectedItem != null)
                {
                    BLE_Device device = DevicesDropdown.SelectedItem as BLE_Device;
                    return (device != null);
                }
                return false;
            }
        }



        #endregion

        private void AppWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // Start BLE device enumeration
            _bleDeviceManager.Start();

            _pleaseSelectdevice = new BLE_Device("(Select a device)", 0);
            _bleDeviceManager.DiscoveredDevices.Add(_pleaseSelectdevice);
            DevicesDropdown.SelectedValue = _pleaseSelectdevice;
        }



        private void ConnectivityManager_OnDisconnecting(object sender, BLE_Device e)
        {
            StoredAnimations.Clear();
            e.ResponseStringEventOccurred -= Device_ResponseStringEventOccurred;
            NotifyPropertyChanged("IsSelectedItemConnected", "IsDeviceRenamable");
        }



        private void AppWindow_Activated(object sender, EventArgs e)
        {
            IsApplicationActive = true;
        }



        private void AppWindow_Deactivated(object sender, EventArgs e)
        {
            IsApplicationActive = false;
        }



        private void SysMenuCloseButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }



        private void DevicesDropdown_SelectionChanged(object sender, RoutedEventArgs e)
        {
            BLE_Device device = DevicesDropdown.SelectedItem as BLE_Device;

            if (_pleaseSelectdevice != null && device != _pleaseSelectdevice)
                _bleDeviceManager.DiscoveredDevices.Remove(_pleaseSelectdevice);
            NotifyPropertyChanged("IsConnectButtonEnabled");
        }



        private async void BrightnessTimer_Tick(object sender, EventArgs e)
        {
            _brightnessTimer.Stop();
            if(_bleDevice != null)
                await _bleDevice.SendChannelString("{\"BRIGHTNESS\", " + CurrentBrightness + "}");
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

                    // Retrieve animations stored on the device
                    await _bleDevice.SendChannelString("{\"CMD\", \"ENUMERATE\"}");
                }
                else
                {
                    // A coonection error occurred so flash an appropriate message to the user
                    Connecting = false;
                    Storyboard sbFlashConnectionError = FindResource("FlashConnectionError") as Storyboard;
                    sbFlashConnectionError.Begin();
                }
                Connecting = false;
            }
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
            catch(Exception ex) { }
            Debug.WriteLine(e);
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



        private async void OnPlayAnimation(object param)
        {
            string animationName = param as string;
            await _bleDevice.SendChannelString("{\"PLAY\", \"" + animationName + "\"}");
        }
     
    }
}
