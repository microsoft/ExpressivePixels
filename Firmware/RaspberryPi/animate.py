import os
import psutil
import random
import subprocess
import yaml

from animation import Animation
from fire import Fire
from rpi_ws281x import Adafruit_NeoPixel, Color
from time import sleep

NUM_ROWS = 18
NUM_COLS = 18

# LED strip configuration:
LED_COUNT = NUM_ROWS * NUM_COLS     # Number of LED pixels.
LED_PIN = 18                        # GPIO pin connected to the pixels (must support PWM!).
LED_FREQ_HZ = 800000                # LED signal frequency in hertz (usually 800khz)
LED_DMA = 10                        # DMA channel to use for generating signal (try 10)
LED_INVERT = False                  # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL    = 0                  # set to '1' for GPIOs 13, 19, 41, 45 or 53

UNASSIGNED = -1

EPX_TOPOLOGY = [
	UNASSIGNED, UNASSIGNED,	UNASSIGNED, UNASSIGNED,	UNASSIGNED, UNASSIGNED, 0, 1, 2, 3, 4, 5, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED,	UNASSIGNED, 
	UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, 
	UNASSIGNED, UNASSIGNED, UNASSIGNED, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, UNASSIGNED, UNASSIGNED, UNASSIGNED, 	
	UNASSIGNED, UNASSIGNED, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, UNASSIGNED, UNASSIGNED, 
	UNASSIGNED, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, UNASSIGNED, 	
	UNASSIGNED, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, UNASSIGNED, 
	74,	75,	76, 77,	78,	79,	80,	81,	82,	83,	84,	85,	86,	87,	88,	89,	90,	91,	
	92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
	110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
	146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
	164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
	UNASSIGNED, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, UNASSIGNED, 	
	UNASSIGNED, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, UNASSIGNED, 	
	UNASSIGNED, UNASSIGNED,214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, UNASSIGNED, UNASSIGNED, 	
	UNASSIGNED, UNASSIGNED, UNASSIGNED, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, UNASSIGNED, UNASSIGNED, UNASSIGNED, 
	UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, 
	UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, 250, 251, 252, 253, 254, 255, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED, UNASSIGNED,UNASSIGNED
]

TOPOLOGY = [
    0, 1, 2, 3, 4, 5, 
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
    41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28,
    42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
    73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58,
    74,	75,	76, 77,	78,	79,	80,	81,	82,	83,	84,	85,	86,	87,	88,	89,	90,	91,
    109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128,
    146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
    181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164,
    182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
    213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198,
    214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227,
    239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249,
    255, 254, 253, 252, 251, 250
]

class Animator:
    def __init__(self):
        self._kill_previous_process()
        self._animations_dir = os.path.join(os.getcwd(), 'animations')
        self._frame_handlers = {
            ord('P'): self._on_pframe,
            ord('I'): self._on_iframe,
            ord('D'): self._on_dframe,
            ord('F'): self._on_fframe
        }
        settings = self._load_settings()
        brightness = settings['brightness'] * 255 // 50
        self._strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, brightness, LED_CHANNEL)
        self._strip.begin()
        self._strip.setBrightness(50)
        self._animation = None

    def _load_settings(self):
        with open('settings.yaml') as fh:
            settings = yaml.load(fh, Loader=yaml.FullLoader)
            return settings

    def _get_previous_process(self):
        cur_pid = psutil.Process().pid
        for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
            if proc.pid == cur_pid or proc.name() != 'python' or 'animate.py' not in proc.cmdline():
                continue
            return proc
        return None

    def _kill_previous_process(self):
        proc = self._get_previous_process()
        if proc:
            proc.terminate()
            psutil.wait_procs([proc])

    def _on_pframe(self, frame):
        for item in frame.data:
            clr = self._animation.palette[item.palette_index]
            color = Color(clr.g, clr.r, clr.b)
            index = TOPOLOGY[EPX_TOPOLOGY[item.pixel_index]]
            if index > 0:
                self._strip.setPixelColor(index, color)
        return

    def _on_iframe(self, frame):
        for pixel_index, palette_index in enumerate(frame.data):
            clr = self._animation.palette[palette_index]
            color = Color(clr.g, clr.r, clr.b)
            index = TOPOLOGY[EPX_TOPOLOGY[pixel_index]]
            if index > 0:
                self._strip.setPixelColor(index, color)
        return

    def _on_dframe(self, frame):
        sleep(frame.delay/1000)

    def _on_fframe(self, frame):
        # TODO: implement proper fading
        sleep(frame.delay)

    def _show_frame(self, frame):
        handler = self._frame_handlers[frame.type]
        handler(frame)
        self._strip.show()

    def _run(self):
        if self._animation == None:
            return
        self.clear()
        loop_count = max(self._animation.loop_count, 1)
        try:
            for i in range(loop_count):
                for frame in self._animation.data:
                    self._show_frame(frame)
                    sleep(0.1)
        except KeyboardInterrupt:
            pass
        self.clear()


    def _load_animations(self):
        files = [f for f in os.listdir(self._animations_dir)]
        animations = []
        for file in files:
            filepath = os.path.join(self._animations_dir, file)
            animation = Animation.load(filepath)
            animations.append(animation)
        return animations

    def _find_animation_by_id(self, id):
        animations = self._load_animations()
        animation = next((x for x in animations if x.id == id), None)
        return animation

    def _find_animation_by_name(self, name):
        animations = self._load_animations()
        animation = next((x for x in animations if x.name == name), None)
        return animation

    def clear(self):
        color = Color(0, 0, 0)
        for i in range(LED_COUNT):
            self._strip.setPixelColor(i, color)
        self._strip.show()

    def play(self, filename):
        self._animation = Animation.load(filename)
        self._run()

    def play_by_id(self, id):
        self._animation = self._find_animation_by_id(id)
        self._run()

    def play_by_name(self, name):
        self._animation = self._find_animation_by_name(name)
        self._run()

    def restart(self):
        proc = self._get_previous_process()
        if not proc:
            return
        args = ['sudo', proc.name()]
        args.extend(proc.cmdline())
        proc.terminate()
        psutil.wait_procs([proc])
        subprocess.run(args)

    def play_all(self, randomly, forever):
        animations = self._load_animations()
        if randomly:
            random.shuffle(animations)
        
        while True:
            for animation in animations:
                print(f'{animation.name}')
                self._animation = animation
                self._run()
            
            if not forever:
                break

            if randomly:
                random.shuffle(animations)

def play_by_id(animation_id):
    animator = Animator()
    animator.play_by_id(animation_id)

def clear_display():
    animator = Animator()
    animator.clear()

    
if __name__ == "__main__":
    Fire(Animator)
