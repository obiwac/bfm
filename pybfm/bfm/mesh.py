from .libbfm import lib, ffi
from .state import default_state

import functools

class Mesh:
	SIMPLEX = 3
	QUAD = 4

	def __init__(self, dim: int, kind: int):
		self.c_mesh = ffi.new("bfm_mesh_t*")
		assert not lib.bfm_mesh_create_generic(self.c_mesh, default_state, dim, kind)

		self.dim = dim
		self.kind = kind

	def __del__(self):
		assert not lib.bfm_mesh_destroy(self.c_mesh)

	def rect(self, first: tuple[float], second: tuple[float], cut: bool = False):
		...

	def mesh(self):
		...
	
	@property
	def edges(self):
		return self.c_mesh.edges

	@functools.cached_property
	def coords(self):
		coords = []

		for i in range(self.c_mesh.n_nodes * self.dim):
			coords.append(self.c_mesh.coords[i] / 2)

		return coords

	def __indices_simplex(self):
		indices = []

		for i in range(self.c_mesh.n_elems * self.kind):
			indices.append(self.c_mesh.elems[i])

		return indices

	def __indices_quad(self):
		indices = []

		# OpenGL doesn't know what quads are
		# so they have to be turned into two triangles

		for i in range(0, self.c_mesh.n_elems):
			# first triangle

			indices.append(self.c_mesh.elems[i * 4 + 0])
			indices.append(self.c_mesh.elems[i * 4 + 1])
			indices.append(self.c_mesh.elems[i * 4 + 3])

			# second triangle

			indices.append(self.c_mesh.elems[i * 4 + 1])
			indices.append(self.c_mesh.elems[i * 4 + 2])
			indices.append(self.c_mesh.elems[i * 4 + 3])

		return indices

	@functools.cached_property
	def indices(self):
		if self.kind == Mesh.QUAD:
			return self.__indices_quad()

		return self.__indices_simplex()

class Mesh_lepl1110(Mesh):
	def __init__(self, name: str):
		self.c_mesh = ffi.new("bfm_mesh_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_mesh_read_lepl1110(self.c_mesh, default_state, c_str)

		self.dim = self.c_mesh.dim
		self.kind = self.c_mesh.kind
