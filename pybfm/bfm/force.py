from .libbfm import lib, ffi
from .state import default_state
from .vec import Vec

# TODO create proper bfm_vec_t objects instead of passing tuples, which obviously can't work

class Force:
	def __init__(self, dim: int):
		self.c_force = ffi.new("bfm_force_t*")
		assert not lib.bfm_force_create(self.c_force, default_state, dim)

	def __del__(self):
		assert not lib.bfm_force_destroy(self.c_force)

# different kinds of forces

class Force_none(Force):
	def __init__(self, dim: int):
		super().__init__(dim)
		assert not lib.bfm_force_set_none(self.c_force)

class __Force_linear(Force):
	def __init__(self, vec: tuple[float]):
		dim = len(vec)
		super().__init__(dim)

		c_vector = Vec(vec)
		assert not lib.bfm_force_set_linear(self.c_force, c_vector.c_vec)

	def __init_subclass__(cls):
		cls.EARTH_GRAVITY = cls((0, -9.81, 0))
		cls.EARTH_GRAVITY_2D = cls((0, -9.81))

class Force_linear(__Force_linear):
	... # to call __init_subclass__

# TODO Force_funky
