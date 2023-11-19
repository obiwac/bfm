from .condition import Condition
from .libbfm import lib, ffi
from .obj import Obj
from .shader import Shader
from .state import default_state

import ctypes
import functools
import math

import pyglet.gl as gl

class CInstance:
	def __init__(self, c_instance, obj: Obj):
		self.c_instance = c_instance
		self.obj = obj

		self.gen_buffers()

	def add_condition(self, condition: Condition):
		assert not lib.bfm_instance_add_condition(self.c_instance, condition.c_condition)

	# visualisation stuff

	def gen_buffers(self):
		# create VAO

		self.vao = gl.GLuint(0)
		gl.glGenVertexArrays(1, ctypes.byref(self.vao))
		gl.glBindVertexArray(self.vao)

		# create coords VBO

		self.vbo = gl.GLuint(0)
		gl.glGenBuffers(1, ctypes.byref(self.vbo))
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.vbo)

		coords_t = gl.GLfloat * len(self.obj.coords)

		gl.glBufferData(gl.GL_ARRAY_BUFFER,
			ctypes.sizeof(coords_t),
			(coords_t) (*self.obj.coords),
			gl.GL_STATIC_DRAW)

		gl.glVertexAttribPointer(0, 3, gl.GL_FLOAT, gl.GL_FALSE, 0, 0)
		gl.glEnableVertexAttribArray(0)

		# create effects VBO
		# XXX this is a separate VBO because we'd like to be able to update this independently of the VBO responsible for rendering the mesh itself

		self.effects_vbo = gl.GLuint(0)
		gl.glGenBuffers(1, ctypes.byref(self.effects_vbo))
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.effects_vbo)

		gl.glVertexAttribPointer(1, 2, gl.GL_FLOAT, gl.GL_FALSE, 0, 0)
		gl.glEnableVertexAttribArray(1)

		# create IBO

		self.ibo = gl.GLuint(0)
		gl.glGenBuffers(1, self.ibo)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.ibo)

		indices_t = gl.GLuint * len(self.obj.indices)

		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER,
			ctypes.sizeof(indices_t),
			(indices_t) (*self.obj.indices),
			gl.GL_STATIC_DRAW)

		# create lines IBO

		self.lines_ibo = gl.GLuint(0)
		gl.glGenBuffers(1, self.lines_ibo)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.lines_ibo)

		line_indices_t = gl.GLuint * len(self.obj.line_indices)

		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER,
			ctypes.sizeof(line_indices_t),
			(line_indices_t) (*self.obj.line_indices),
			gl.GL_STATIC_DRAW)

	@functools.cached_property
	def effects(self):
		effects = []

		for i in range(self.c_instance.n_effects):
			effects.append(self.c_instance.effects[i])

		# *2 for both faces

		return 2 * effects

	@functools.cached_property
	def max_effect(self):
		max_effect = 0

		for i in range(self.obj.mesh.c_mesh.n_nodes):
			x = self.c_instance.effects[i * 2 + 0]
			y = self.c_instance.effects[i * 2 + 1]

			effect = math.sqrt(x ** 2 + y ** 2)
			max_effect = max(max_effect, effect)

		return max_effect

	def update_effects(self):
		gl.glBindVertexArray(self.vao)
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.effects_vbo)

		effects_t = gl.GLfloat * len(self.effects)

		gl.glBufferData(gl.GL_ARRAY_BUFFER,
			ctypes.sizeof(effects_t),
			(effects_t) (*self.effects),
			gl.GL_STATIC_DRAW)

	def draw(self, shader: Shader, lines = False):
		shader.max_effect(self.max_effect)
		gl.glBindVertexArray(self.vao)

		if lines:
			gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.lines_ibo)
			gl.glDrawElements(gl.GL_LINES, len(self.obj.line_indices), gl.GL_UNSIGNED_INT, None)

		else:
			gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.ibo)
			gl.glDrawElements(gl.GL_TRIANGLES, len(self.obj.indices), gl.GL_UNSIGNED_INT, None)

	def export_js(self) -> str:
		return f"""{{
			indices: new Uint32Array({self.obj.indices}),
			line_indices: new Uint32Array({self.obj.line_indices}),
			coords: new Float32Array({self.obj.coords}),
			effects: new Float32Array({self.effects}),
			max_effect: {self.max_effect},
		}}"""

class Instance(CInstance):
	def __init__(self, obj: Obj):
		c_instance = ffi.new("bfm_instance_t*")
		assert not lib.bfm_instance_create(c_instance, default_state, obj.c_obj)

		super().__init__(c_instance, obj)

	def __del__(self):
		# TODO maybe delete buffers idk
		assert not lib.bfm_instance_destroy(self.c_instance)
