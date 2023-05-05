from .libbfm import lib, ffi
from .state import default_state

# TODO create proper bfm_vec_t objects instead of passing tuples, which obviously can't work

class __Force:
	def __init__(self, dim: int):
		self.c_force = ffi.new("bfm_force_t*")
		assert not lib.bfm_force_create(self.c_force, default_state, dim)

	def __del__(self):
		assert not lib.bfm_force_destroy(self.c_force)

	def eval(self, pos: tuple[float]) -> tuple[float]:
		assert not lib.bfm_force_eval(self.c_force, pos, None) # TODO force_ref - how???

# different kinds of forces

class Force_none(__Force):
	def __init__(self, dim: int):
		super().__init__(dim)
		assert not lib.bfm_force_set_none(self.c_force)

class __Force_linear(__Force):
	def __init__(self, dim: int, vec: tuple[float]):
		super().__init__(dim)
		assert not lib.bfm_force_set_linear(self.c_force, vec)

	def __init_subclass__(cls):
		cls.EARTH_GRAVITY = cls(3, (0, -9.81, 0))

class Force_linear(__Force_linear):
	... # to call __init_subclass__

# TODO Force_funky
