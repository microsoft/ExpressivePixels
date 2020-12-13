import pickle

from collections import namedtuple
from datetime import datetime
from struct import unpack
from uuid import uuid4

Color = namedtuple('Color', 'r g b')
PFrameData = namedtuple('PFrameData', 'pixel_index palette_index')
IFrame = namedtuple('IFrame', 'type count data')
PFrame = namedtuple('PFrame', 'type count data')
DFrame = namedtuple('DFrame', 'type delay')
FFrame = namedtuple('FFrame', 'type fade')


class Animation:
    @staticmethod
    def load(filename):
        with open(filename, 'rb') as fh:
            animation = pickle.load(fh)
            return animation
    
    @staticmethod    
    def save(filename, animation):
        with open(filename, 'wb') as fh:
            pickle.dump(animation, fh)
        return

    def __init__(self, data):
        offset = 0

        if not data:
            self.id = str(uuid4()).replace('-', '')
            self.utc = int(datetime.utcnow().timestamp()) 
            self.data = []
            self.palette = []
            self.frame_rate = 20
            self.frames_count = 0
            self.loop_count = 1
            self.name = ''
            return

        animation_id = ''.join(list(f'{ch:02x}' for ch in data[:16]))
        offset += 16

        # header
        utc, offset = self._read_uint32(data, offset)
        frames_size, offset = self._read_uint32(data, offset)

        palette_count, offset = self._read_uint16(data, offset)
        frame_count, offset = self._read_uint16(data, offset)

        frame_rate, offset = self._read_uint8(data, offset)
        loop_count, offset = self._read_uint8(data, offset)
        name_length, offset = self._read_uint8(data, offset)
        
        pad, offset = self._read_uint8(data, offset)

        name = ''.join(chr(ch) for ch in data[offset : offset + name_length])
        offset += name_length

        # palette bytes
        palette = []
        for i in range(palette_count):
            r, offset = self._read_uint8(data, offset)
            g, offset = self._read_uint8(data, offset)
            b, offset = self._read_uint8(data, offset)
            palette.append(Color(r, g, b))
        
        # frame_data = data[offset:]
        frames = []
        for i in range(frame_count):
            frame = None
            frame_type, offset = self._read_uint8(data, offset)
            if frame_type == ord('I'):
                count, offset = self._read_uint16_be(data, offset)
                frame_data = []
                for j in range(count):
                    index, offset = self._read_uint8(data, offset)
                    frame_data.append(index)
                frame = IFrame(frame_type, count, frame_data)
            elif frame_type == ord('P'):
                count, offset = self._read_uint16_be(data, offset)
                frame_data = []
                for j in range(count):
                    pixel_index, offset = self._read_uint16_be(data, offset)
                    palette_index, offset = self._read_uint8(data, offset)
                    frame_data.append(PFrameData(pixel_index, palette_index))
                frame = PFrame(frame_type, count, frame_data)
            elif frame_type == ord('D'):
                delay, offset = self._read_uint16_be(data, offset)
                frame = DFrame(frame_type, delay)
            elif frame_type == ord('F'):
                fade, offset = self._read_uint16_be(data, offset)
                frame = FFrame(frame_type, fade)
            else:
                raise ("Unknown frame type")
            frames.append(frame)

        self.id = animation_id
        self.utc = utc 
        self.data = frames
        self.palette = palette
        self.frame_rate = frame_rate
        self.frames_count = frame_count
        self.loop_count = max(loop_count, 1)
        self.name = name

    def _read_value(self, data, offset, size, fmt):
        value = unpack(fmt, data[offset:offset + size])[0]
        offset += size
        return value, offset

    def _read_uint8(self, data, offset):
        return self._read_value(data, offset, 1, 'B')

    def _read_uint16(self, data, offset):
        return self._read_value(data, offset, 2, '<h')

    def _read_uint16_be(self, data, offset):
        return self._read_value(data, offset, 2, '>h')

    def _read_uint32(self, data, offset):
        return self._read_value(data, offset, 4, '<I')

