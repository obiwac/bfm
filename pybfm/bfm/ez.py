from .instance import CInstance
from .libbfm import lib, ffi
from .material import CMaterial
from .mesh import Mesh
from .obj import CObj
from .rule import CRule
from .sim import CSim
from .state import default_state

class Ez_lepl1110:
	def __init__(self, mesh: Mesh, name: str):
		self.c_ez = ffi.new("bfm_ez_lepl1110_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_ez_lepl1110_create(self.c_ez, default_state, mesh.c_mesh, c_str)

		self.mesh = mesh
		self.material = CMaterial(ffi.addressof(self.c_ez.material))
		self.rule = CRule(ffi.addressof(self.c_ez.rule))
		self.obj = CObj(ffi.addressof(self.c_ez.obj), self.mesh, self.material, self.rule)
		self.instance = CInstance(ffi.addressof(self.c_ez.instance), self.obj)
		self.sim = CSim(ffi.addressof(self.c_ez.sim), [self.instance], self.c_ez.sim.kind)

	def write(self, filename, shift):
		c_str = ffi.new("char[]", bytes(filename, "utf-8"))
		assert not lib.bfm_ez_lepl1110_write(self.c_ez, shift, c_str)
