from .material import Material
from .mesh import Mesh

import ctypes

import pyglet.gl as gl

class Obj:
	def __init__(self, mesh: Mesh, material: Material):
		self.mesh = mesh
		self.material = material

		self.gen_buffers()

	def gen_buffers(self):
		# create VAO

		self.vao = gl.GLuint(0)
		gl.glGenVertexArrays(1, ctypes.byref(self.vao))
		gl.glBindVertexArray(self.vao)

		# create VBO

		self.vbo = gl.GLuint(0)
		gl.glGenBuffers(1, ctypes.byref(self.vbo))
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.vbo)

		coords_t = gl.GLfloat * len(self.mesh.coords)

		gl.glBufferData(gl.GL_ARRAY_BUFFER,
			ctypes.sizeof(coords_t),
			(coords_t) (*self.mesh.coords),
			gl.GL_STATIC_DRAW)

		gl.glVertexAttribPointer(0, self.mesh.dim, gl.GL_FLOAT, gl.GL_FALSE, 0, 0)
		gl.glEnableVertexAttribArray(0)

		# create IBO

		self.ibo = gl.GLuint(0)
		gl.glGenBuffers(1, self.ibo)
		gl.glBindBuffer(gl.GL_ELEMENT_ARRAY_BUFFER, self.ibo)

		indices_t = gl.GLuint * len(self.mesh.indices)

		gl.glBufferData(gl.GL_ELEMENT_ARRAY_BUFFER,
			ctypes.sizeof(indices_t),
			(indices_t) (*self.mesh.indices),
			gl.GL_STATIC_DRAW)

	def draw(self):
		gl.glBindVertexArray(self.vao)

		if self.mesh.kind == Mesh.SIMPLEX:
			mode = gl.GL_TRIANGLES

		elif self.mesh.kind == Mesh.QUAD:
			mode = gl.GL_QUADS

		gl.glDrawElements(mode, len(self.mesh.indices), gl.GL_UNSIGNED_INT, None)
