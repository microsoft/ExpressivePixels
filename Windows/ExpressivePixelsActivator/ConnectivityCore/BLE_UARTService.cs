using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Storage.Streams;



namespace Microsoft.Research.EmbeddedDeviceConnectivity
{
    public interface IGattService
    {
        string Name { get; }
    }



    public interface IUARTService : IGattService
    {
        bool IsServiceStarted { get; }
        Task<bool> Start(GattDeviceService uARTService, bool stringChannel);        
        event UARTService.RXReceivedHandler RXReceived;
        Task Stop();
    }



    public class UARTService : IUARTService
    {
        private const int BLEPAYLOADSIZE=237;

        #region Events
        public delegate void RXReceivedHandler(byte[] payload);
        public event RXReceivedHandler RXReceived;

        public delegate void TransmitCompletionHandler(double completion);
        public event TransmitCompletionHandler TransmitCompletion;
        #endregion

        public UARTService()
        {
        }



        public async Task<bool> Start(GattDeviceService uARTService, bool stringChannel = false)
        {
            if (uARTService == null || uARTService.Uuid != GattServiceUuids.UART)
            {
                iSServiceStarted = false;
                return iSServiceStarted;
            }
            else
            {
                UARTGattService = uARTService;
                GattCharacteristicsResult result = await this.UARTGattService.GetCharacteristicsForUuidAsync(stringChannel ? GattCharacteristicsUuid.ARX : GattCharacteristicsUuid.RX);
                rxCharacteristic = result.Characteristics.FirstOrDefault();

                iSServiceStarted = await EnableRXNotification();
                return iSServiceStarted;
            }
        }



        public async Task Stop()
        {
            if (iSServiceStarted)
            {
                try
                { 
                UARTGattService = null;
                await DisableRXNotification();
                rxCharacteristic.Service.Dispose();
                rxCharacteristic = null;
                }
                catch { }
            }
        }



        public async Task<bool> SendBuffer(byte [] bufferSource, bool stringChannel = false)
        {
            try
            {
                double currentCompletion = 0, lastCompletion = 0;
                int completionDelta = bufferSource.Length / 10;

                if (!iSServiceStarted || bufferSource == null)
                    return false;

                GattCharacteristicsResult result = await this.UARTGattService.GetCharacteristicsForUuidAsync(stringChannel ? GattCharacteristicsUuid.ATX : GattCharacteristicsUuid.TX);
                var txCharacteristic = result.Characteristics.FirstOrDefault();

                int bufferOffset = 0;
                while (bufferOffset < bufferSource.Length)
                {
                    int chunkSize = Math.Min(BLEPAYLOADSIZE, bufferSource.Length - bufferOffset);
                    var ibuffer = ToIBuffer(bufferSource, bufferOffset, chunkSize);
                    await txCharacteristic.WriteValueAsync(ibuffer, GattWriteOption.WriteWithoutResponse);
                    bufferOffset += chunkSize;

                    currentCompletion = (double) ((int) bufferOffset / (int)completionDelta) / 10;
                    if (currentCompletion > lastCompletion)
                    {
                        Debug.WriteLine("BLEUartService::SendBuffer SENT " + bufferOffset);
                        lastCompletion = currentCompletion;
                        if (TransmitCompletion != null)
                            TransmitCompletion(currentCompletion);
                    }
                }
                return true;
            }
            catch (Exception e)
            {
                Debug.WriteLine("BLEUartService::SendBuffer EXCEPTION " + e.ToString());
                return false;
            }
        }



        private async Task<bool> EnableRXNotification()
        {
            if (rxCharacteristic != null)
            {
                rxCharacteristic.ValueChanged += rxCharacteristic_ValueChanged;
                var currentStatus = await rxCharacteristic.ReadClientCharacteristicConfigurationDescriptorAsync();

                GattCommunicationStatus result = await rxCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.Notify);
                return result == GattCommunicationStatus.Success;
            }
            return false;
        }



        private async Task DisableRXNotification()
        {
            if (rxCharacteristic != null)
            {
                try
                {
                    rxCharacteristic.ValueChanged -= rxCharacteristic_ValueChanged;
                    GattCommunicationStatus status = await rxCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.None);
                }
                catch
                {
                }
            }
        }

        

        void rxCharacteristic_ValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args)
        {
            if (RXReceived != null)
            {
                var data = new byte[args.CharacteristicValue.Length];
                DataReader.FromBuffer(args.CharacteristicValue).ReadBytes(data);
                RXReceived(data);
            }
        }



        public string Name
        {
            get
            {
                return GattServiceNames.UART;
            }
        }


        private bool iSServiceStarted = false;
        public bool IsServiceStarted
        {
            get
            {
                return iSServiceStarted;
            }
        }
        private GattDeviceService UARTGattService { get; set; }

        public GattCharacteristic rxCharacteristic { get; set; }

        public static IBuffer ToIBuffer(byte[] value, int offset, int copyLength)
        {
            if (value == null && copyLength == 0)
                throw new ArgumentException();
            var temp = new byte[copyLength];
            Array.Copy(value, offset, temp, 0, copyLength);
            using (DataWriter writer = new DataWriter())
            {
                writer.WriteBytes(temp);
                var buffer = writer.DetachBuffer();
                return buffer;
            }
        }

    }
}
