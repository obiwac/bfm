from .mesh import Mesh

import ctypes
import functools

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

	def gen_buffers(self):
		# create VAO

		self.vao = gl.GLuint(0)
		gl.glGenVertexArrays(1, ctypes.byref(self.vao))
		gl.glBindVertexArray(self.vao)

		# create coords VBO

		self.vbo = gl.GLuint(0)
		gl.glGenBuffers(1, ctypes.byref(self.vbo))
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.vbo)

		coords_t = gl.GLfloat * len(self.coords)

		gl.glBufferData(gl.GL_ARRAY_BUFFER,
			ctypes.sizeof(coords_t),
			(coords_t) (*self.coords),
			gl.GL_STATIC_DRAW)

		gl.glVertexAttribPointer(0, 3, gl.GL_FLOAT, gl.GL_FALSE, 0, 0)
		gl.glEnableVertexAttribArray(0)

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
