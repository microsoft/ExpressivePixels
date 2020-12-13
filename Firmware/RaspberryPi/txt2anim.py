import fire
from PIL import Image, ImageDraw, ImageFont
from animation import Animation, IFrame, DFrame, Color

def get_palette(images):
    palette = []
    for image in images:
        w, h = image.size
        colors = [image.getpixel((i % w, i // w)) for i in range(w*h)]
        palette.extend(colors)
    palette = list(set(palette))
    return palette

def images_to_animation(images, delay):
    animation = Animation(None)
    palette = get_palette(images)
    palette_lookup  = {clr: index for index, clr in enumerate(palette)}
    animation.palette = [Color(r, g, b) for (r, g, b) in palette]

    frames = []
    for image in images:
        w, h = image.size
        colors = [image.getpixel((i % w, i // w)) for i in range(w*h)]
        indices = [palette_lookup[clr] for clr in colors]
        frame = IFrame(ord('I'), w * h, indices)
        frames.append(frame)

        frame = DFrame(ord('D'), delay)
        frames.append(frame)

    animation.frames_count = len(frames)
    animation.data = frames
    return animation    

def text_to_animation(text, font_size, color, delay):
    animation_size = (18, 18)
    font_path = '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf'
    font = ImageFont.truetype(font_path, font_size)
    width, height = animation_size
    images = []
    for char in text:
        img = Image.new('RGB', animation_size)
        draw = ImageDraw.Draw(img)
        size = draw.textsize(char, font)
        x = (width - size[0]) // 2
        y = (height - size[1]) // 2
        draw.text((x, y), char, color, font)
        images.append(img)
    animation = images_to_animation(images, delay)
    animation.name = text
    Animation.save(text, animation)


# e.g. 
# python txt2anim.py "HAPPY NEW YEAR" 15, #00ff00 500 
if __name__ == '__main__':
    fire.Fire(text_to_animation)


