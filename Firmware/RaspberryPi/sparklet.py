import fire
import  logging 
import os
import tempfile
import yaml

from datetime import datetime

from animate import Animator, play_by_id, clear_display
from animation import Animation
from ble_device import BleDevice

from multiprocessing import Process, Queue
from protocol import EPXProtocol, EPXCommand, ProtocolFormat
from serial_device import SerialDevice
from signal import signal, SIGINT
from sys import exit

from collections import namedtuple

DISPLAYWIDTH = 18
DISPLAYHEIGHT = 18

class Sparklet:
    def __init__(self):
        signal(SIGINT, self.on_sigint)
        self.animations_dir = os.path.join(os.getcwd(), 'animations')
        os.makedirs(self.animations_dir, exist_ok=True)
        self.protocol = EPXProtocol()
        self.animator_process = None

        self.command_handlers = [
            self.on_connect_headerrq,
            self.on_clear_display,
            self.on_display_brightness,
            self.on_enumerate_animations,
            self.on_preview_color,
            self.on_upload_frame8,
            self.on_upload_animation8,
            self.on_upload_pixel8,
            self.on_remove_animation,
            self.on_store_animation,
            self.on_play_stored_animation8,
            self.on_play_stored_animation8_byname,
            self.on_set_device_name,
            self.on_request_thumbnail,
            self.on_set_key,
            self.on_console_command
        ]
        with open('settings.yaml', 'r') as fh:
            self.settings = yaml.load(fh, Loader=yaml.FullLoader)
        #self.start_animation_process()

    def start_animation_process(self):
        #self.animator = Animator()
        self.frames_queue = Queue()
        self.count = 0
        args = (self.frames_queue, )
        self.animator_process = Process(target=play_frames, args=args)
        self.animator_process.start()

    def stop_animation_process(self):
        if not self.animator_process:
            return

    def run_animation_process(self, target, args):
        if self.animator_process:
            self.animator_process.terminate()
            self.animator_process.join()
        self.animator_process = Process(target=target, args=args)
        self.animator_process.start()

    def ble(self):
        self.device = BleDevice(self)
        logging.info('Starting Sparklet')
        self.device.start()
        logging.info('Hit <ENTER> to disconnect')
        input()
        self.device.stop()
        self.stop_animation_process()
        logging.info('Sparklet stopped')

    def serial(self):
        self.device = SerialDevice(self)
        logging.info('Starting Sparklet')
        self.device.start()
        self.stop_animation_process()
        logging.info('Sparklet stopped')

        
    def on_sigint(self, signum, frame):
        self.device.stop()
        logging.info('Sparklet stopped')
        exit(0)
    
    def on_read_notify(self):
        logging.info(f'Sparklet.on_read_notify')

    def on_read_request(self, offset):
        logging.info(f'Sparklet.on_read_request {offset}')

    def on_write_request(self, data, offset):
        logging.debug(f'Sparklet.on_write_request received {len(data)} bytes at {datetime.now().strftime("%m/%d/%Y-%I:%M:%S.%f")}')
    
        commands, send_ack = self.protocol.decode(data)
        for command in commands:
            header, payload = command
            logging.info(f'Sparklet.on_write_request {EPXCommand(header.command)} - {header.transaction_id}')
            logging.debug(f'Sparklet.on_write_request {payload}')

            handler = self.command_handlers[header.command - 1]
            success = handler(header, payload)

        if send_ack:
            frames = self.protocol.encode_binaryack()
            logging.debug(f'Sparklet.on_write_request sending BINARY ACK at {datetime.now().strftime("%m/%d/%Y-%I:%M:%S.%f")}')
            self._send_frames(frames)
        return True

    def _send_frames(self, frames):
        for frame in frames:
            logging.debug(f'Sparklet.on_enumerate_animations sent {len(frame)} bytes')
            self.device.write(frame)
        return True        

    def on_connect_headerrq(self, command, payload):
        data = {
            'status': 'success',
            'TransactionID': command.transaction_id,
            'data': {
                'FIRMWARE': '0.1',
                'DEVICENAME': self.device.name,
                'CAPABILITIES': ['STORAGE'], #'PREVIEW', 
                'BATTERY': { 'PCT': 100 },
                'DISPLAYWIDTH': DISPLAYWIDTH,
                'DISPLAYHEIGHT': DISPLAYHEIGHT,
                'AUTOPLAY': 1,
                'BRIGHTNESS': self.settings['brightness']
            },
        }

        frames = self.protocol.encode(data, ProtocolFormat.JSON.value, 0)
        return self._send_frames(frames)

    def on_clear_display(self, header, payload):
        self.run_animation_process(clear_display, ())
        return True
    
    def on_display_brightness(self, header, payload):
        self.settings['brightness'] = payload.Brightness
        with open('settings.yaml', 'w') as fh:
            yaml.dump(self.settings, fh)
        return True
    
    def get_animations_list(self):
        files = [f for f in os.listdir(self.animations_dir)]
        animations = []
        for file in files:
            filepath = os.path.join(self.animations_dir, file)
            st = os.stat(filepath)
            
            animation = Animation.load(filepath)

            item = {}
            item['ID'] =  animation.id
            item['Filename'] = animation.name
            item['Size'] = st.st_size
            item['UTC'] = animation.utc

            animations.append(item)
        return animations

    def on_enumerate_animations(self, header, payload):
        df = os.statvfs('/')
        space_used = df.f_bfree / df.f_blocks * 100
        
        data = {
            'status': 'success',
            'TransactionID': payload.TransactionID,
            'StorageUsed': space_used,
            'Sequences': self.get_animations_list()
        }

        frames = self.protocol.encode(data, ProtocolFormat.JSON.value, 0)
        return self._send_frames(frames)

    def on_preview_color(self, header, payload):
        pass
    
    def on_upload_frame8(self, header, payload):
        # TODO: 
        # upload_frame requires a serious amount of CPU 
        # to support live preview. The support for interprocess
        # communication is a bit weak on Python and shared memory
        # support is not coming till Python3.8 which is not yet 
        # supported on Raspberry Pi
        
        # self.frames_queue.put(payload.data)
        # self.count += 1
        # logging.info(f'put={self.frames_queue.qsize()}')
        return True
    
    def on_upload_animation8(self, header, payload):
        pass
    
    def on_upload_pixel8(self, header, payload):
        pass
    
    def on_remove_animation(self, header, payload):
        files = [f for f in os.listdir(self.animations_dir)]
        for file in files:
            filepath = os.path.join(self.animations_dir, file)
            animation = Animation.load(filepath)
            if animation.id == payload.ID:
                os.remove(filepath)
                break
        return True

    def on_store_animation(self, header, payload):
        filename = os.path.join(self.animations_dir, payload.name)
        Animation.save(filename, payload)
        return True
    
    def on_play_stored_animation8(self, header, payload):
        self.run_animation_process(play_by_id, (payload.ID,))
        return True
    
    def on_play_stored_animation8_byname(self, header, payload):
        self.run_animation_process(play_by_id, (payload.Name,))
        return True
    
    def on_set_device_name(self, header, payload):
        pass
    
    def on_request_thumbnail(self, header, payload):
        pass
        
    def on_set_key(self, header, payload):
        pass
    
    def on_console_command(self, header, payload):
        pass
    


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    fire.Fire(Sparklet)