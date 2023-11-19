from .mesh import Mesh
from .util import jsify_list

import ctypes
import functools
import math

import pyglet.gl as gl

class Scenery:
	def __init__(self, mesh: Mesh):
		self.mesh = mesh
		self.gen_buffers()

	@property
	def __c_mesh(self):
		return self.mesh.c_mesh

	@functools.cached_property
	def coords(self):
		return self.mesh.coords

	@functools.cached_property
	def __indices_simplex(self):
		indices = []
		elems = self.__c_mesh.elems

		for i in range(self.__c_mesh.n_elems * self.mesh.dim):
			indices.append(elems[i])

		return indices

	@functools.cached_property
	def __indices_quad(self):
		indices = []
		elems = self.__c_mesh.elems

		# OpenGL doesn't know what quads are
		# so they have to be turned into two triangles

		for i in range(self.__c_mesh.n_elems):
			# first triangle

			indices.append(elems[i * 4 + 0])
			indices.append(elems[i * 4 + 1])
			indices.append(elems[i * 4 + 3])

			# second triangle

			indices.append(elems[i * 4 + 1])
			indices.append(elems[i * 4 + 2])
			indices.append(elems[i * 4 + 3])

		return indices

	@functools.cached_property
	def indices(self):
		if self.mesh.kind == Mesh.QUAD:
			return self.__indices_quad

		return self.__indices_simplex

	@functools.cached_property
	def normals(self):
		normals = [0] * self.__c_mesh.n_nodes * self.mesh.dim

		elems = self.__c_mesh.elems
		coords = self.__c_mesh.coords

		for i in range(self.__c_mesh.n_elems):
			# only need three vertices to calculate element normal

			a = elems[i * self.mesh.kind + 0]
			b = elems[i * self.mesh.kind + 1]
			c = elems[i * self.mesh.kind + 2]

			# only need two vectors to calculate element normal

			ab = (
				coords[a * self.mesh.dim + 0] - coords[b * self.mesh.dim + 0],
				coords[a * self.mesh.dim + 1] - coords[b * self.mesh.dim + 1],
				coords[a * self.mesh.dim + 2] - coords[b * self.mesh.dim + 2],
			)

			ac = (
				coords[a * self.mesh.dim + 0] - coords[c * self.mesh.dim + 0],
				coords[a * self.mesh.dim + 1] - coords[c * self.mesh.dim + 1],
				coords[a * self.mesh.dim + 2] - coords[c * self.mesh.dim + 2],
			)

			# take the cross product between ab and ac

			n = (
				 ab[1] * ac[2] - ab[2] * ac[1],
				-ab[0] * ac[2] + ab[2] * ac[0],
				 ab[0] * ac[1] - ab[1] * ac[0],
			)

			# add normal to each vertex

			for j in range(self.mesh.kind):
				vertex_i = elems[i * self.mesh.kind + j]

				for k, component in enumerate(n):
					normals[vertex_i * self.mesh.dim + k] += component

		# normalize the normals

		for i in range(self.__c_mesh.n_nodes):
			norm_squared = 0

			for j in range(self.mesh.dim):
				norm_squared += normals[i * self.mesh.dim + j] ** 2

			norm = math.sqrt(norm_squared)

			for j in range(self.mesh.dim):
				normals[i * self.mesh.dim + j] /= norm

		return normals

	@functools.cached_property
	def vbo_data(self):
		n = len(self.coords) + len(self.normals)
		data = [0] * n

		for i in range(0, len(self.coords), 3):
			data[i * 2 + 0] = self.coords[i + 0]
			data[i * 2 + 1] = self.coords[i + 1]
			data[i * 2 + 2] = self.coords[i + 2]

			data[i * 2 + 3] = self.normals[i + 0]
			data[i * 2 + 4] = self.normals[i + 1]
			data[i * 2 + 5] = self.normals[i + 2]

		return data

	def gen_buffers(self):
		# create VAO

		self.vao = gl.GLuint(0)
		gl.glGenVertexArrays(1, ctypes.byref(self.vao))
		gl.glBindVertexArray(self.vao)

		# create VBO

		self.vbo = gl.GLuint(0)
		gl.glGenBuffers(1, ctypes.byref(self.vbo))
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.vbo)

		data_t = gl.GLfloat * len(self.vbo_data)
		float_size = ctypes.sizeof(gl.GLfloat)

		gl.glBufferData(gl.GL_ARRAY_BUFFER,
			ctypes.sizeof(data_t),
			(data_t) (*self.vbo_data),
			gl.GL_STATIC_DRAW)

		gl.glVertexAttribPointer(0, 3, gl.GL_FLOAT, gl.GL_FALSE, float_size * 6, 0)
		gl.glEnableVertexAttribArray(0)

		gl.glVertexAttribPointer(1, 3, gl.GL_FLOAT, gl.GL_FALSE, float_size * 6, float_size * 3)
		gl.glEnableVertexAttribArray(1)

		# create IBO

		self.ibo = gl.GLuint(0)
		gl.glGenBuffers(1, self.ibo)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.ibo)

		indices_t = gl.GLuint * len(self.indices)

		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER,
			ctypes.sizeof(indices_t),
			(indices_t) (*self.indices),
			gl.GL_STATIC_DRAW)

	def draw(self):
		gl.glBindVertexArray(self.vao)
		gl.glDrawElements(gl.GL_TRIANGLES, len(self.indices), gl.GL_UNSIGNED_INT, None)

	def export_js(self) -> str:
		return f"""{{
			indices: new Uint32Array({jsify_list(self.indices)}),
			vbo_data: new Float32Array({jsify_list(self.vbo_data)}),
		}}"""
