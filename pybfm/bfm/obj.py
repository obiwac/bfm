from .libbfm import lib, ffi
from .material import Material
from .mesh import Mesh
from .rule import Rule
from .state import default_state

class CObj:
	def __init__(self, c_obj, mesh: Mesh, material: Material, rule: Rule):
		self.c_obj = c_obj

		self.mesh = mesh
		self.material = material
		self.rule = rule

class Obj(CObj):
	def __init__(self, mesh: Mesh, material: Material, rule: Rule):
		c_obj = ffi.new("bfm_obj_t*")
		assert not lib.bfm_obj_create(c_obj, default_state, mesh.c_mesh, material.c_material, rule.c_rule)

		super().__init__(c_obj, mesh, material, rule)

	def __del__(self):
		assert not lib.bfm_obj_destroy(self.c_obj)
