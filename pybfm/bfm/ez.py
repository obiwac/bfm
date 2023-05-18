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

		self.material = CMaterial(self.c_ez.material)
		self.rule = CRule(self.c_ez.rule)
		self.obj = CObj(self.c_ez.obj, mesh, material, rule)
		self.instance = CInstance(self.c_ez.instance, obj)
		self.sim = CSim(self.c_ez.sim, [self.instance], ISim.DEFORMATION)
