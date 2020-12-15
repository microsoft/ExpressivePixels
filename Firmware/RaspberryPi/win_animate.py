"""
Array Backed Grid

Show how to use a two-dimensional list/array to back the display of a
grid on-screen.

Note: Regular drawing commands are slow. Particularly when drawing a lot of
items, like the rectangles in this example.

For faster drawing, create the shapes and then draw them as a batch.
See array_backed_grid_buffered.py

If Python and Arcade are installed, this example can be run from the command line with:
python -m arcade.examples.array_backed_grid
"""
import arcade
import fire
import time


from animation import Animation, Color

# Set how many rows and columns we will have
ROW_COUNT = 18
COLUMN_COUNT = 18

# This sets the WIDTH and HEIGHT of each grid location
WIDTH = 30
HEIGHT = 30

# This sets the margin between each cell
# and on the edges of the screen.
MARGIN = 5

# Do the math to figure out our screen dimensions
SCREEN_WIDTH = (WIDTH + MARGIN) * COLUMN_COUNT + MARGIN
SCREEN_HEIGHT = (HEIGHT + MARGIN) * ROW_COUNT + MARGIN
SCREEN_TITLE = "Expressive Pixels Animation"


class AnimationWindow(arcade.Window):
    """
    Main application class.
    """

    def __init__(self, width, height, title):
        super().__init__(width, height, title)

        # Create a 2 dimensional array. A two dimensional
        # array is simply a list of lists.
        self.grid = []
        for row in range(ROW_COUNT):
            # Add an empty array that will hold each cell
            # in this row
            self.grid.append([])
            for column in range(COLUMN_COUNT):
                self.grid[row].append(0)  # Append a cell

        arcade.set_background_color(arcade.color.BLACK)

        self.frame_handlers = {
            ord('P'): self.on_pframe,
            ord('I'): self.on_iframe,
            ord('D'): self.on_dframe,
            ord('F'): self.on_fframe
        }
        self.delay = None

    def load(self, filename):
        self.animation = Animation.load(filename)
        self.cur_frame = 0

    def draw_grid(self):
        for row in range(ROW_COUNT):
            for column in range(COLUMN_COUNT):
                # Do the math to figure out where the box is
                x = (MARGIN + WIDTH) * column + MARGIN + WIDTH // 2
                y = (MARGIN + HEIGHT) * row + MARGIN + HEIGHT // 2
                color = self.grid[row][column]
                color = (color.r, color.g, color.b) if type(color) == Color else (0, 0, 0)

                # Draw the box
                arcade.draw_rectangle_filled(x, y, WIDTH, HEIGHT, color)
        return

    def update_grid(self):
        frame = self.animation.data[self.cur_frame]
        handler = self.frame_handlers[frame.type]
        handler(frame)
        self.cur_frame = (self.cur_frame + 1) % len(self.animation.data)

    def on_draw(self):
        if self.delay:
            if time.time() < self.delay:
                return
            else:
                self.delay = None

        # This command has to happen before we start drawing
        arcade.start_render()
        self.update_grid()
        # Draw the grid
        self.draw_grid()

    def on_pframe(self, frame):
        for item in frame.data:
            row = (ROW_COUNT - 1) - (item.pixel_index // ROW_COUNT)
            col = item.pixel_index % ROW_COUNT
            self.grid[row][col] = self.animation.palette[item.palette_index]
        return

    def on_iframe(self, frame):
        for i, palette_index in enumerate(frame.data):
            row = (ROW_COUNT - 1) - (i // ROW_COUNT)
            col = i % ROW_COUNT
            self.grid[row][col] = self.animation.palette[palette_index]
        return

    def on_dframe(self, frame):
        self.delay = time.time() + (frame.delay / 1000)

    def on_fframe(self, frame):
        self.delay = time.time() + (frame.delay / 1000)


def animate(filename):
    window = AnimationWindow(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE)
    window.load(filename)
    arcade.run()


if __name__ == "__main__":
    fire.Fire(animate)