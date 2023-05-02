import pyglet

pyglet.options["shadow_window"] = False
pyglet.options["debug_gl"] = False

import pyglet.gl as gl

from .obj import Obj

class Window(pyglet.window.Window):
	def __init__(self, **args):
		super().__init__(**args)
		pyglet.clock.schedule_interval(self.update, 1.0 / 60)

	def update(self, dt):
		...

	def on_draw(self):
		gl.glEnable(gl.GL_DEPTH_TEST)
		# gl.glEnable(gl.GL_CULL_FACE)

		gl.glClearColor(0.0, 0.0, 0.0, 0.0)
		self.clear()

	def on_resize(self, width, height):
		print(f"Resize {width} * {height}")
		gl.glViewport(0, 0, width, height)

	def on_mouse_press(self, x, y, button, modifiers):
		...

	def on_mouse_motion(self, x, y, delta_x, delta_y):
		...

	def on_mouse_drag(self, x, y, delta_x, delta_y, buttons, modifiers):
		self.on_mouse_motion(x, y, delta_x, delta_y)

	def on_key_press(self, key, modifiers):
		if key == pyglet.window.key.ESCAPE:
			pyglet.app.exit()

	def on_key_release(self, key, modifiers):
		...

class Bfm:
	def __init__(self):
		self.config = gl.Config(major_version = 3, minor_version = 3, depth_size = 16)
		self.window = Window(config = self.config, width = 800, height = 480, caption = "BFM", resizable = True, vsync = False)

	def add(self, obj: Obj):
		...

	def show(self):
		pyglet.app.run()

if __name__ == "__main__":
	bfm = Bfm()
	bfm.show()
