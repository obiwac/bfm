from .libbfm import lib, ffi
from .state import default_state

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

class Mesh_lepl1110(Mesh):
	def __init__(self, name: str):
		self.c_mesh = ffi.new("bfm_mesh_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_mesh_read_lepl1110(self.c_mesh, default_state, c_str)

		self.dim = self.c_mesh.dim
		self.kind = self.c_mesh.kind

class Mesh_wavefront(Mesh):
	def __init__(self, name: str, full: bool = False):
		self.c_mesh = ffi.new("bfm_mesh_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_mesh_read_wavefront(self.c_mesh, default_state, c_str, full)

		self.dim = self.c_mesh.dim
		self.kind = self.c_mesh.kind
