from cobs import cobs
from collections import namedtuple
from dataclasses import dataclass
from enum import Enum
from struct import pack, unpack
from types import SimpleNamespace

import array
import logging
from pybleno.hci_socket.Io import writeUInt8, readUInt8, readUInt16LE
import json

from animation import Animation

MAX_BLE_PAYLOAD_SIZE = 237
COBS_MAXFRAME_SIZE = 255
MAX_COBS_DATAFRAMESIZE = 225

class ProtocolMode(Enum):
    COBS = 0
    STRING = 1

class ProtocolFormat(Enum):
    BINARY = 0
    JSON = 1
    BINARYACK = 2 
    STRING = 3    

class Flags(Enum):
    ACKRQ = 1

class FrameType(Enum):
    HEADERPLUSDATA = 0
    DATA = 1

class HeaderSizes(Enum):
    PRIMARYHEADER = 5
    DATAHEADER = 1

class EPXCommand(Enum):
    CONNECT_HEADERRQ = 1
    CLEAR_DISPLAY = 2
    DISPLAY_BRIGHTNESS = 3
    ENUMERATE_ANIMATIONS = 4 
    PREVIEW_COLOR = 5
    UPLOAD_FRAME8 = 6
    UPLOAD_ANIMATION8 = 7
    UPLOAD_PIXEL8 = 8
    REMOVE_ANIMATION = 9
    STORE_ANIMATION8 = 10
    PLAY_STORED_ANIMATION8 = 11
    PLAY_STORED_ANIMATION8_BYNAME = 12
    SETDEVICENAME = 13
    REQUEST_THUMBNAIL = 14 
    SETKEY = 15
    CONSOLE_COMMAND = 16 

DeviceProtocolHeader = namedtuple('DeviceProtocolHeader', 'frame_type format length flags')

EPXAppProtocolHeader = namedtuple('EPXAppProtocolHeader', 'transaction_id command')
EPXAppProtocolHeader_format_str = '<IH'

DevicePixelPayload = namedtuple('DevicePixelPayload', 'transaction_id command pixel_index r g b pad')
DevicePixelPayload_format_str = '<IHHBBBB'

DeviceFramePayload = namedtuple('DeviceFramePayload', 'width height data')

ANIMATION_HEADER_SIZE = 38
        
class EPXProtocol:
    def __init__(self):
        self.staging_buffer = array.array('B', [0] * COBS_MAXFRAME_SIZE)
        self.output_format = ProtocolFormat.BINARY
        self.reset()

    def reset(self):
        self.output_frame_initialized = False
        self.output_bytes_remaining = 0
        self.output_buffer = None
        self.staging_index = 0

    def decode(self, payload):
        commands = []

        payload_index = 0

        while payload_index < len(payload):
            self.staging_buffer[self.staging_index] = payload[payload_index]
            self.staging_index += 1
            payload_index += 1
            
            if self.staging_buffer[self.staging_index - 1] == 0:
                try:
                    buffer = cobs.decode(self.staging_buffer[:self.staging_index - 1])
                except:
                    print(f'cobs.decode encountered an exception')
                    self.reset()
                    return commands, False

                logging.debug(f'EPXProtocol.decode decoded {len(buffer)} bytes')

                if len(buffer) > 0:
                    if buffer[0] == FrameType.HEADERPLUSDATA.value:
                        offset = HeaderSizes.PRIMARYHEADER.value
                        device_header = DeviceProtocolHeader._make(unpack('<BBHB', buffer[:offset]))
                        self.output_format = device_header.format
                        self.output_buffer = bytearray(buffer[offset:])
                        self.output_bytes_remaining = device_header.length - len(self.output_buffer)
                        logging.debug(f'EPXProtocol.decode found HEADER with {len(self.output_buffer)} bytes. Need {self.output_bytes_remaining} bytes more')
                    elif buffer[0] == FrameType.DATA.value and self.output_buffer != None:
                        offset = HeaderSizes.DATAHEADER.value
                        data = buffer[offset:]
                        self.output_buffer.extend(data)
                        self.output_bytes_remaining -= len(data)
                        logging.debug(f'EPXProtocol.decode found DATA with {len(self.output_buffer)} bytes. Need {self.output_bytes_remaining} bytes more')

                    if self.output_bytes_remaining == 0:
                        header, data = self._decode_command(self.output_buffer)
                        commands.append((header, data))
                        self.output_buffer = None
                        self.staging_index = 0

                self.staging_index = 0
            
            if self.staging_index == COBS_MAXFRAME_SIZE:
                self.reset()
        
        send_ack = self.staging_index > 0 or self.output_bytes_remaining > 0
        return commands, send_ack         

    def _decode_command(self, output):
        if self.output_format == ProtocolFormat.JSON.value:
            data =  ''.join(chr(x) for x in output)
            command = json.loads(data, object_hook=lambda d: SimpleNamespace(**d))
            app_header = EPXAppProtocolHeader(command.TransactionID, EPXCommand[command.Command].value)
            return app_header, command
        else:
            app_header = EPXAppProtocolHeader._make(unpack(EPXAppProtocolHeader_format_str, output[:6]))
            if app_header.command == EPXCommand.STORE_ANIMATION8.value:
                animation = Animation(output[6:])
                return app_header, animation
            elif app_header.command == EPXCommand.UPLOAD_FRAME8.value:
                # TODO: Fix all the format strings and constants for offsets
                width = unpack('<H', output[6:8])[0]
                height = unpack('<H', output[8:10])[0]
                data = output[10:10 + (width * height * 3)]
                frame = DeviceFramePayload(width, height, data)
                return app_header, frame
        return None, None

    def encode(self, payload, format, flags):
        if format == ProtocolFormat.JSON.value:
            payload = json.dumps(payload)
            payload = [ord(ch) for ch in payload]

        frame_type = FrameType.HEADERPLUSDATA.value
        frames = []
        remaining = len(payload)
        cur_offset = 0
        header_size = HeaderSizes.PRIMARYHEADER.value

        while cur_offset < remaining:
            staging_frame_size = min(MAX_COBS_DATAFRAMESIZE, header_size + remaining - cur_offset)
            staging_frame = bytearray([0] * staging_frame_size)
            frame = bytearray([0] * (staging_frame_size + 1 + 1))
            
            staging_frame[0] = frame_type
            if (cur_offset == 0):
                staging_frame[1] = format
                staging_frame[2] = len(payload) & 0xFF
                staging_frame[3] = (len(payload) >> 8) & 0xFF
                staging_frame[4] = flags
            
            to_copy = min(staging_frame_size - header_size, len(payload))
            staging_frame[header_size : header_size + to_copy] = payload[cur_offset : cur_offset + to_copy]
            cur_offset += to_copy

            cobs_buf = cobs.encode(staging_frame)
            frame = bytearray(cobs_buf)
            frame.append(0)

            frames.append(frame)

            frame_type = FrameType.DATA.value
            header_size = HeaderSizes.DATAHEADER.value
        return frames

    def encode_binaryack(self):
        magic_ack = array.array('B', [0xA1, 0x85, 0x29, 0xB8, 0xBC, 0xED, 0xFE, 0x62]);
        frames = self.encode(magic_ack, ProtocolFormat.BINARY.value, 0)
        return frames
