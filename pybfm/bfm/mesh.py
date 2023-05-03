from .libbfm import lib, ffi

from .state import default_state

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

class Mesh_lepl1110(Mesh):
	def __init__(self, name: str):
		self._mesh = ffi.new("bfm_mesh_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		lib.bfm_mesh_read_lepl1110(self._mesh, default_state, c_str)
