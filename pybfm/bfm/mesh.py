from .libbfm import lib, ffi

from .state import default_state

class Mesh:
	SIMPLEX = 3
	QUAD = 4

	def __init__(self, dim: int, kind: int):
		self.mesh = ffi.new("bfm_mesh_t*")
		lib.bfm_mesh_create_generic(self.mesh, default_state, dim, kind)

		self.dim = dim
		self.kind = kind

	def __del__(self):
		lib.bfm_mesh_destroy(self.mesh)

	def rect(self, first: tuple[float], second: tuple[float], cut: bool = False):
		...

	def mesh(self):
		...

class Mesh_lepl1110(Mesh):
	def __init__(self, name: str):
		self.mesh = ffi.new("bfm_mesh_t*")
		lib.bfm_mesh_read_lepl1110(self.mesh, default_state, name)
