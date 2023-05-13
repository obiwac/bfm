from .libbfm import lib, ffi
from .material import Material
from .mesh import Mesh
from .rule import Rule
from .state import default_state

# TODO move these to separate files (package resource)

class Obj:
	def __init__(self, mesh: Mesh, material: Material, rule: Rule):
		self.c_obj = ffi.new("bfm_obj_t*")
		assert not lib.bfm_obj_create(self.c_obj, default_state, mesh.c_mesh, material.c_material, rule.c_rule)

		self.mesh = mesh
		self.material = material
		self.rule = rule

	def __del__(self):
		assert not lib.bfm_obj_destroy(self.c_obj)
