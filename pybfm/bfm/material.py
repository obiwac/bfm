from .libbfm import lib, ffi
from .state import default_state

class CMaterial:
	def __init__(self, c_material, colour: tuple[float] = (1, 1, 1, 1)):
		self.c_material = c_material
		assert not lib.bfm_material_set_colour(self.c_material, *colour)

	@property
	def name(self):
		return ffi.string(self.c_material.name)

	def __repr__(self):
		return f"BFM_Material({name})"

	def __init_subclass__(cls):
		cls.AA7075 = cls("AA7075", 2.81e3, 71.7e9, 0.33, (0.667, 0.439, 0.459, 1))

class Material(CMaterial):
	def __init__(self, name: str, rho: float, E: float, nu: float, colour: tuple[float] = (1, 1, 1, 1)):
		c_material = ffi.new("bfm_material_t*")

		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_material_create(c_material, default_state, c_str, rho, E, nu)

		super().__init__(c_material, colour)

	def __del__(self):
		assert not lib.bfm_material_destroy(self.c_material)
