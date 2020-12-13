import array
from pybleno.hci_socket.Io import writeUInt8, copy

from pybleno import Bleno, BlenoPrimaryService, Characteristic, Descriptor


def get_sparklet_advertisement_data(short_name, name, serviceUuids):
    flags_len = 3
    appearance_len = 4
    advertisementDataLength = flags_len + appearance_len
    scanDataLength = 0

    serviceUuids128bit = []
    i = 0

    advertisementDataLength += 2 + len(short_name)
    scanDataLength += 2 + len(name)

    for i,uuid in enumerate(serviceUuids):
        serviceUuid = bytearray.fromhex(uuid)  
        serviceUuid.reverse()
        serviceUuids128bit.append(serviceUuid)
    
    advertisementDataLength += 2 + 16 * len(serviceUuids128bit)

    advertisementData = array.array('B', [0] * advertisementDataLength)

    scanData = array.array('B', [0] * scanDataLength)

    # flags
    writeUInt8(advertisementData, 0x02, 0)
    writeUInt8(advertisementData, 0x01, 1)
    writeUInt8(advertisementData, 0x06, 2)

    advertisementDataOffset = 3

    # appearance
    writeUInt8(advertisementData, 0x03, 3)
    writeUInt8(advertisementData, 0x19, 4)
    writeUInt8(advertisementData, 0xC6, 5)
    writeUInt8(advertisementData, 0x07, 6)
    advertisementDataOffset += 4

    writeUInt8(advertisementData, 1 + 16 * len(serviceUuids128bit), advertisementDataOffset)
    advertisementDataOffset += 1

    writeUInt8(advertisementData, 0x06, advertisementDataOffset)
    advertisementDataOffset += 1

    for i,uuid in enumerate(serviceUuids128bit):
        copy(uuid, advertisementData, advertisementDataOffset)
        advertisementDataOffset += len(uuid)


    # name in ad data
    nameBuffer = array.array('B', [ord(elem) for elem in short_name])
    writeUInt8(advertisementData, 1 + len(nameBuffer), advertisementDataOffset)
    writeUInt8(advertisementData, 0x08, advertisementDataOffset + 1)
    copy(nameBuffer, advertisementData, advertisementDataOffset + 2)

    # name in scan data
    nameBuffer = array.array('B', [ord(elem) for elem in name])
    writeUInt8(scanData, 1 + len(nameBuffer), 0)
    writeUInt8(scanData, 0x08, 1)
    copy(nameBuffer, scanData, 2)

    return advertisementData, scanData

    

class UartTx(Characteristic):
    def __init__(self, on_write_request):
        options = {
            'uuid': "6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
            'properties': ['notify', 'writeWithoutResponse'],
            'descriptors': [
                Descriptor({'uuid': '02', 'value': 'Write data'})
            ],
            'onWriteRequest': on_write_request,
        }
        self.update_value_callback = None
        super().__init__(options) 


class UartRx(Characteristic):
    def __init__(self, on_read_notify, on_read_request):
        options = {
            'uuid': "6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
            'properties': ['notify', 'read'],
            'descriptors': [
                Descriptor({'uuid': '03', 'value': 'Read data'})
            ],
            'onNotify': on_read_notify,
            'onReadRequest': on_read_request,
            'onSubscribe': self.onSubscribe,
            'onUnsubscribe': self.onUnsubscribe,
        }
        super().__init__(options) 

    def onSubscribe(self, max_value_size, update_value_callback):
        self.max_value_size = max_value_size
        self.update_value_callback = update_value_callback

    def onUnsubscribe(self):
        self.max_value_size = None
        self.update_value_callback = None


class UartService(BlenoPrimaryService):
    def __init__(self, on_read_notify, on_read_request, on_write_request):
        self.tx = UartTx(on_write_request)
        self.rx = UartRx(on_read_notify, on_read_request)
        options = {
            'uuid': '6E400001-B5A3-F393-E0A9-E50E24DCCA9E',
            'characteristics':[
                self.tx,
                self.rx
            ]
        }
        super().__init__(options)


class BleDevice:
    def __init__(self, sparklet):
        self.name = 'PiSpark'
        self.short_name = 'PSPK'
        self.sparklet = sparklet
        self.uart = UartService(
            self.on_read_notify,
            self.on_read_request,
            self.on_write_request
        )
        self.ad_data, self.scan_data = get_sparklet_advertisement_data(self.short_name, self.name, [self.uart.uuid])
        self.ble = Bleno()
        self.ble.on('advertisingStart', self.on_advertising_start)
        self.ble.on('stateChange', self.on_state_change)
        self.ble.on('accept', self.on_accept)

    def on_state_change(self, state):
        print(f'Sparklet.on_state_change: {state}')
        if state == 'poweredOn':
            self.ble.startAdvertisingWithEIRData(self.ad_data, self.scan_data, callback=self.on_start_advertising)
        return

    def on_read_notify(self):
        success = self.sparklet.on_read_notify()

    def on_read_request(self, offset):
        success = self.sparklet.on_read_request(offset)

    def on_write_request(self, data, offset, withoutResponse, callback):
        if self.sparklet.on_write_request(data, offset):
            callback(Characteristic.RESULT_SUCCESS)
        return

    def write(self, data):
        self.uart.rx.update_value_callback(data)

    def on_start_advertising(self, error):
        print(f'Sparklet.on_start_advertising: {error}')

    def on_advertising_start(self, error):
        print(f'Sparklet.on_advertising_start: {error}')
        self.ble.setServices([self.uart])

    def on_accept(self, address):
        print(f'Sparklet.on_accept: {address}')
    
    def start(self):
        self.ble.start()

    def stop(self):
        self.ble.stopAdvertising()
        self.ble.disconnect()
