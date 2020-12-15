import logging
import serial
import time

class SerialDevice:
    def __init__(self, sparklet):
        self.sparklet = sparklet
        self.name = 'PiSpark'
        self.short_name = 'PSPK'
        self.port = '/dev/ttyS0'
        self.baud = 115200
        self.stop = False
        self.timeout = 10

    def start(self):
        self.serial = serial.Serial(self.port, self.baud)
        self.serial.timeout = 10
        while not self.stop:
            data = self.serial.read(4096)
            if len(data) > 0:
                logging.debug(f'SerialDevice received {len(data)} bytes')
                self.sparklet.on_write_request(data, 0)
            else:
                time.sleep(0.1)
        self.serial.close()

    def write(self, data):
        logging.debug(f'SerialDevice writing {len(data)} bytes')
        self.serial.write(data)

    def stop(self):
        self.stop = True


