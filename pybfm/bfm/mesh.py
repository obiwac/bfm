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

		for i in range(self.c_mesh.n_nodes):
			coords.append(self.c_mesh.coords[i * 2 + 0])
			coords.append(self.c_mesh.coords[i * 2 + 1])
			coords.append(-0.1)

		for i in range(self.c_mesh.n_nodes):
			coords.append(self.c_mesh.coords[i * 2 + 0])
			coords.append(self.c_mesh.coords[i * 2 + 1])
			coords.append(0.1)

		return coords

	@functools.cached_property
	def __indices_simplex(self):
		indices = []

		n = self.c_mesh.n_nodes
		elems = self.c_mesh.elems

		for i in range(self.c_mesh.n_elems):
			indices.append(elems[i * 3 + 0])
			indices.append(elems[i * 3 + 1])
			indices.append(elems[i * 3 + 2])

		for i in range(self.c_mesh.n_elems):
			indices.append(n + elems[i * 3 + 0])
			indices.append(n + elems[i * 3 + 1])
			indices.append(n + elems[i * 3 + 2])

		return indices

	@functools.cached_property
	def __indices_quad(self):
		indices = []

		n = self.c_mesh.n_nodes
		elems = self.c_mesh.elems

		# OpenGL doesn't know what quads are
		# so they have to be turned into two triangles

		for i in range(self.c_mesh.n_elems):
			# first triangle

			indices.append(elems[i * 4 + 0])
			indices.append(elems[i * 4 + 1])
			indices.append(elems[i * 4 + 3])

			# second triangle

			indices.append(elems[i * 4 + 1])
			indices.append(elems[i * 4 + 2])
			indices.append(elems[i * 4 + 3])

		for i in range(self.c_mesh.n_elems):
			# first triangle

			indices.append(n + elems[i * 4 + 0])
			indices.append(n + elems[i * 4 + 1])
			indices.append(n + elems[i * 4 + 3])

			# second triangle

			indices.append(n + elems[i * 4 + 1])
			indices.append(n + elems[i * 4 + 2])
			indices.append(n + elems[i * 4 + 3])

		return indices

	@functools.cached_property
	def indices(self):
		if self.kind == Mesh.QUAD:
			indices = self.__indices_quad

		else:
			indices = self.__indices_simplex

		n = self.c_mesh.n_nodes

		# connect edges of two faces

		for i in range(self.c_mesh.n_edges):
			edge = self.c_mesh.edges[i]

			# only count boundary edges

			if edge.elems[1] >= 0:
				continue

			a = edge.nodes[0]
			b = edge.nodes[1]

			# first triangle

			indices.append(a)
			indices.append(b)
			indices.append(a + n)

			# second triangle

			indices.append(b)
			indices.append(b + n)
			indices.append(a + n)

		return indices

	@functools.cached_property
	def __line_indices_simplex(self):
		indices = []

		n = self.c_mesh.n_nodes
		elems = self.c_mesh.elems

		for i in range(self.c_mesh.n_elems):
			# first line

			indices.append(elems[i * 3 + 0])
			indices.append(elems[i * 3 + 1])

			# second line

			indices.append(elems[i * 3 + 1])
			indices.append(elems[i * 3 + 2])

			# third line

			indices.append(elems[i * 3 + 2])
			indices.append(elems[i * 3 + 0])

		for i in range(self.c_mesh.n_elems):
			# first line

			indices.append(n + elems[i * 3 + 0])
			indices.append(n + elems[i * 3 + 1])

			# second line

			indices.append(n + elems[i * 3 + 1])
			indices.append(n + elems[i * 3 + 2])

			# third line

			indices.append(n + elems[i * 3 + 2])
			indices.append(n + elems[i * 3 + 0])

		return indices

	@functools.cached_property
	def __line_indices_quad(self):
		indices = []

		n = self.c_mesh.n_nodes
		elems = self.c_mesh.elems

		for i in range(self.c_mesh.n_elems):
			# first line

			indices.append(elems[i * 4 + 0])
			indices.append(elems[i * 4 + 1])

			# second line

			indices.append(elems[i * 4 + 1])
			indices.append(elems[i * 4 + 2])

			# third line

			indices.append(elems[i * 4 + 2])
			indices.append(elems[i * 4 + 3])

			# fourth line

			indices.append(elems[i * 4 + 3])
			indices.append(elems[i * 4 + 0])

		for i in range(self.c_mesh.n_elems):
			# first line

			indices.append(n + elems[i * 4 + 0])
			indices.append(n + elems[i * 4 + 1])

			# second line

			indices.append(n + elems[i * 4 + 1])
			indices.append(n + elems[i * 4 + 2])

			# third line

			indices.append(n + elems[i * 4 + 2])
			indices.append(n + elems[i * 4 + 3])

			# fourth line

			indices.append(n + elems[i * 4 + 3])
			indices.append(n + elems[i * 4 + 0])

		return indices

	@functools.cached_property
	def line_indices(self):
		if self.kind == Mesh.QUAD:
			return self.__line_indices_quad

		return self.__line_indices_simplex

class Mesh_lepl1110(Mesh):
	def __init__(self, name: str):
		self.c_mesh = ffi.new("bfm_mesh_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_mesh_read_lepl1110(self.c_mesh, default_state, c_str)

		self.dim = self.c_mesh.dim
		self.kind = self.c_mesh.kind

class Mesh_wavefront(Mesh):
	def __init__(self, name: str):
		self.c_mesh = ffi.new("bfm_mesh_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_mesh_read_wavefront(self.c_mesh, default_state, c_str)

		self.dim = self.c_mesh.dim
		self.kind = self.c_mesh.kind
