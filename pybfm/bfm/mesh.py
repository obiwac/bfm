from .libbfm import lib, ffi
from .state import default_state

import ctypes

import pyglet.gl as gl

class Mesh:
	SIMPLEX = 3
	QUAD = 4

	def __init__(self, dim: int, kind: int):
		self._mesh = ffi.new("bfm_mesh_t*")
		lib.bfm_mesh_create_generic(self._mesh, default_state, dim, kind)

		self.dim = dim
		self.kind = kind

	def __del__(self):
		lib.bfm_mesh_destroy(self._mesh)

	def rect(self, first: tuple[float], second: tuple[float], cut: bool = False):
		...

	def mesh(self):
		...

	@property
	def coords(self):
		coords = []

		for i in range(self._mesh.n_nodes * self.dim):
			coords.append(self._mesh.coords[i] / 500)

		return coords

	@property
	def indices(self):
		indices = []

		for i in range(self._mesh.n_elems):
			indices.append(self._mesh.elems[i])

		return indices

	def gen_buffers(self):
		# create VAO

		self.vao = gl.GLuint(0)
		gl.glGenVertexArrays(1, ctypes.byref(self.vao))
		gl.glBindVertexArray(self.vao)

		# create VBO

		self.vbo = gl.GLuint(0)
		gl.glGenBuffers(1, ctypes.byref(self.vbo))
		gl.glBindBuffer(gl.GL_ARRAY_BUFFER, self.vbo)

		coords_t = gl.GLfloat * len(self.coords)

		gl.glBufferData(gl.GL_ARRAY_BUFFER,
			ctypes.sizeof(coords_t),
			(coords_t) (*self.coords),
			gl.GL_STATIC_DRAW)

		gl.glVertexAttribPointer(0, self.dim, gl.GL_FLOAT, gl.GL_FALSE, 0, 0)
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
		if self.kind == Mesh.SIMPLEX:
			mode = gl.GL_TRIANGLES

		elif self.kind == Mesh.QUAD:
			mode = gl.GL_QUADS

		gl.glDrawElements(mode, len(self.indices), gl.GL_UNSIGNED_INT, None)

class Mesh_lepl1110(Mesh):
	def __init__(self, name: str):
		self._mesh = ffi.new("bfm_mesh_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		lib.bfm_mesh_read_lepl1110(self._mesh, default_state, c_str)

		self.dim = self._mesh.dim
		self.kind = self._mesh.kind
