import html
import pyglet.gl as gl

from .force import Force
from .instance import Instance
from .libbfm import lib, ffi
from .matrix import Matrix
from .shader import Shader
from .state import default_state

class CSim:
	NONE                = 0
	PLANAR_STRAIN       = 1
	PLANAR_STRESS       = 2
	AXISYMMETRIC_STRAIN = 3

	def __init__(self, c_sim, instances: list[Instance], kind: int):
		self.c_sim = c_sim
		self.instances = instances

		# create shader, depending on simulation kind
		# TODO in fine, make these package resources

		self.shader = Shader("shaders/sim/deformation.vert", "shaders/sim/deformation.frag")
		self.line_shader = Shader("shaders/sim/line_deformation.vert", "shaders/sim/deformation.frag")

	def add_instance(self, instance: Instance):
		assert not lib.bfm_sim_add_instance(self.c_sim, instance.c_instance)
		self.instances.append(instance)

	def add_force(self, force: Force):
		assert not lib.bfm_sim_add_force(self.c_sim, force.c_force)

	def run(self):
		assert not lib.bfm_sim_run(self.c_sim)

	# visualisation functions

	def show(self):
		for instance in self.instances:
			instance.update_effects()

	def draw(self, mvp_matrix: Matrix, anim: float):
		gl.glEnable(gl.GL_DEPTH_TEST)

		self.shader.use()

		self.shader.mvp_matrix(mvp_matrix)
		self.shader.anim(anim)

		for instance in self.instances:
			instance.draw(self.shader)

		# draw lines

		self.line_shader.use()

		self.line_shader.mvp_matrix(mvp_matrix)
		self.line_shader.anim(anim)

		for instance in self.instances:
			instance.draw(self.line_shader, True)

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

		src_js = f"{src_matrix_js}{src_js}"

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

class Sim(CSim):
	def __init__(self, kind: int):
		c_sim = ffi.new("bfm_sim_t*")
		assert not lib.bfm_sim_create(c_sim, default_state, kind)

		instances: list[Instance] = []
		super().__init__(c_sim, instances, kind)

	def __del__(self):
		assert not lib.bfm_sim_destroy(self.c_sim)
