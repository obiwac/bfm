import html
import math

import pyglet

pyglet.options["shadow_window"] = False
pyglet.options["debug_gl"] = False

import pyglet.gl as gl

from .matrix import Matrix
from .mesh import Mesh
from .scenery import Scenery
from .shader import Shader
from .sim import Sim

class Window(pyglet.window.Window):
	def __init__(self, **args):
		super().__init__(**args)
		pyglet.clock.schedule_interval(self.update, 1.0 / 60)

		# scene

		self.current_sim: Sim | None = None
		self.scenery: list[Scenery] = []

		self.scenery_shader = Shader("shaders/scenery.vert", "shaders/scenery.frag")

		# orbit camera

		self.default_recoil = 1.
		self.default_rotation = [0, 0]
		self.default_origin = [0, 0, 0]

		self.orbit_defaults(True)

		self.mv_matrix = Matrix()
		self.p_matrix = Matrix()

		self.time = 0
		self.anim_state = 2

	def orbit_defaults(self, set_real = False):
		self.target_recoil = self.default_recoil
		*self.target_rotation, = self.default_rotation
		*self.target_origin, = self.default_origin

		if set_real:
			self.recoil = self.target_recoil
			*self.rotation, = self.target_rotation
			*self.origin, = self.target_origin

	def __anim(self, target, val, dt, speed):
		fac = dt * speed

		if fac > 1:
			return target

		return val + fac * (target - val)

	def update(self, dt):
		self.time += dt

		self.recoil = self.__anim(self.target_recoil, self.recoil, dt, 10)

		self.rotation[0] = self.__anim(self.target_rotation[0], self.rotation[0], dt, 20)
		self.rotation[1] = self.__anim(self.target_rotation[1], self.rotation[1], dt, 20)

		self.origin[0] = self.__anim(self.target_origin[0], self.origin[0], dt, 20)
		self.origin[1] = self.__anim(self.target_origin[1], self.origin[1], dt, 20)
		self.origin[2] = self.__anim(self.target_origin[2], self.origin[2], dt, 20)

	def on_draw(self):
		# create MVP matrix

		self.p_matrix.load_identity()
		self.p_matrix.perspective(90, self.width / self.height, 0.1, 500)

		self.mv_matrix.load_identity()
		self.mv_matrix.translate(0, 0, -pow(self.recoil, 2))
		self.mv_matrix.rotate_2d(*self.rotation)
		self.mv_matrix.translate(*self.origin)

		mvp_matrix = self.p_matrix @ self.mv_matrix

		# set up drawing

		gl.glDisable(gl.GL_CULL_FACE)

		gl.glClearColor(1, 1, 1, 0)
		self.clear()

		# draw scenery

		self.scenery_shader.use()
		self.scenery_shader.mvp_matrix(mvp_matrix)

		for scenery in self.scenery:
			scenery.draw()

		# draw simulation

		anim = math.sin(self.time) / 2 + .5

		if self.current_sim is not None:
			self.current_sim.draw(mvp_matrix, (0, 1, anim)[self.anim_state])

	def on_resize(self, width, height):
		print(f"Resize {width} * {height}")
		gl.glViewport(0, 0, width, height)

	def on_mouse_press(self, x, y, button, modifiers):
		...

	def on_mouse_motion(self, x, y, delta_x, delta_y):
		...

	def on_mouse_drag(self, x, y, delta_x, delta_y, buttons, modifiers):
		self.on_mouse_motion(x, y, delta_x, delta_y)

		# orbiting

		if buttons & pyglet.window.mouse.LEFT:
			self.target_rotation[0] += delta_x / 200
			self.target_rotation[1] += delta_y / 200

			self.target_rotation[1] = max(-math.tau / 4, min(math.tau / 4, self.target_rotation[1]))

		# panning

		if buttons & pyglet.window.mouse.RIGHT:
			self.target_origin[0] += delta_x / 200
			self.target_origin[1] += delta_y / 200

	def on_mouse_scroll(self, x, y, scroll_x, scroll_y):
		self.target_recoil -= scroll_y / 10
		self.target_recoil = max(self.target_recoil, 0.5)

	def on_key_press(self, key, modifiers):
		if key == pyglet.window.key.ESCAPE:
			pyglet.app.exit()

		if key == pyglet.window.key.SPACE:
			self.orbit_defaults()

		if key == pyglet.window.key.C:
			print(f"bfm.set_default_recoil({self.target_recoil})")
			print(f"bfm.set_default_rotation({self.target_rotation})")
			print(f"bfm.set_default_origin({self.target_origin})")

		if key == pyglet.window.key.A:
			self.anim_state = (self.anim_state + 1) % 3

	def on_key_release(self, key, modifiers):
		...

class Bfm:
	def __init__(self):
		try:
			self.config = gl.Config(double_buffer = True, major_version = 3, minor_version = 3, depth_size = 16, sample_buffers = 1, samples = 4)
			self.window = Window(config = self.config, width = 480, height = 480, caption = "BFM", resizable = True, vsync = False)

		except pyglet.window.NoSuchConfigException:
			self.config = gl.Config(double_buffer = True, major_version = 3, minor_version = 3, depth_size = 16)
			self.window = Window(config = self.config, width = 480, height = 480, caption = "BFM (no AA)", resizable = True, vsync = False)

	def set_default_recoil(self, recoil: float):
		self.window.default_recoil = recoil
		self.window.orbit_defaults()

	def set_default_rotation(self, rotation: list[float]):
		*self.window.default_rotation, = rotation
		self.window.orbit_defaults()

	def set_default_origin(self, origin: list[float]):
		*self.window.default_origin, = origin
		self.window.orbit_defaults()

	def add_scenery(self, mesh: Mesh) -> Scenery:
		scenery = Scenery(mesh)
		self.window.scenery.append(scenery)

		return scenery

	def show(self, sim: Sim):
		sim.show()
		self.window.current_sim = sim
		pyglet.app.run()

	# exporting

	def export(self, path="index.html", title="BFM Web Export", width: int=1280, height: int=720):
		def read(path):
			with open(path) as f:
				return f.read()

		# read templates

		src_html = read("web/index.html")
		src_js = read("web/index.js")
		src_matrix_js = read("web/matrix.js")
		src_scenery_vert = read("shaders/scenery.vert")
		src_scenery_frag = read("shaders/scenery.frag")

		# generate JS source

		src_js = f"""
			{src_matrix_js}
			window.addEventListener("load", () => {{
				{src_js}
			}})
		"""

		# generate HTML source

		src_html = src_html.replace("$TITLE", html.escape(title))
		src_html = src_html.replace("$JS_SRC", src_js)

		src_html = src_html.replace("$SCENERY_VERT", src_scenery_vert)
		src_html = src_html.replace("$SCENERY_FRAG", src_scenery_frag)

		src_html = src_html.replace("$WIDTH", str(width))
		src_html = src_html.replace("$HEIGHT", str(height))

		# write output

		with open(path, "w") as f:
			f.write(src_html)
