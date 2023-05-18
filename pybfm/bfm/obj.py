from .libbfm import lib, ffi
from .material import Material
from .mesh import Mesh
from .rule import Rule
from .state import default_state

import functools

class CObj:
	def __init__(self, c_obj, mesh: Mesh, material: Material, rule: Rule):
		self.c_obj = c_obj

		self.mesh = mesh
		self.material = material
		self.rule = rule

	# mesh stuff
	# this is in Obj because the coords/indices returned from here are specific to simulation objects

	@property
	def __c_mesh(self):
		return self.mesh.c_mesh

	@functools.cached_property
	def coords(self):
		return [self.__c_mesh.coords[i] for i in range(self.__c_mesh.n_nodes * self.mesh.dim)]

	@functools.cached_property
	def gl_coords(self):
		coords = []

		for i in range(self.__c_mesh.n_nodes):
			coords.append(self.__c_mesh.coords[i * 2 + 0])
			coords.append(self.__c_mesh.coords[i * 2 + 1])
			coords.append(-0.1)

		for i in range(self.__c_mesh.n_nodes):
			coords.append(self.__c_mesh.coords[i * 2 + 0])
			coords.append(self.__c_mesh.coords[i * 2 + 1])
			coords.append(0.1)

		return coords

	@functools.cached_property
	def __indices_simplex(self):
		indices = []

		n = self.__c_mesh.n_nodes
		elems = self.__c_mesh.elems

		for i in range(self.__c_mesh.n_elems):
			indices.append(elems[i * 3 + 0])
			indices.append(elems[i * 3 + 1])
			indices.append(elems[i * 3 + 2])

		for i in range(self.__c_mesh.n_elems):
			indices.append(n + elems[i * 3 + 0])
			indices.append(n + elems[i * 3 + 1])
			indices.append(n + elems[i * 3 + 2])

		return indices

	@functools.cached_property
	def __indices_quad(self):
		indices = []

		n = self.__c_mesh.n_nodes
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

		for i in range(self.__c_mesh.n_elems):
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
		if self.mesh.kind == Mesh.QUAD:
			indices = self.__indices_quad

		else:
			indices = self.__indices_simplex

		n = self.__c_mesh.n_nodes

		# connect edges of two faces

		for i in range(self.__c_mesh.n_edges):
			edge = self.__c_mesh.edges[i]

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

		n = self.__c_mesh.n_nodes
		elems = self.__c_mesh.elems

		for i in range(self.__c_mesh.n_elems):
			# first line

			indices.append(elems[i * 3 + 0])
			indices.append(elems[i * 3 + 1])

			# second line

			indices.append(elems[i * 3 + 1])
			indices.append(elems[i * 3 + 2])

			# third line

			indices.append(elems[i * 3 + 2])
			indices.append(elems[i * 3 + 0])

		for i in range(self.__c_mesh.n_elems):
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

		n = self.__c_mesh.n_nodes
		elems = self.__c_mesh.elems

		for i in range(self.__c_mesh.n_elems):
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

		for i in range(self.__c_mesh.n_elems):
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
		if self.mesh.kind == Mesh.QUAD:
			return self.__line_indices_quad

		return self.__line_indices_simplex

class Obj(CObj):
	def __init__(self, mesh: Mesh, material: Material, rule: Rule):
		c_obj = ffi.new("bfm_obj_t*")
		assert not lib.bfm_obj_create(c_obj, default_state, mesh.c_mesh, material.c_material, rule.c_rule)

		super().__init__(c_obj, mesh, material, rule)

	def __del__(self):
		assert not lib.bfm_obj_destroy(self.c_obj)
