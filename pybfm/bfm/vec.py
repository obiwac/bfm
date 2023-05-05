from .libbfm import lib, ffi
from .state import default_state

class Vec:
	def __init__(self, vec: tuple[float]):
		self.c_vec = ffi.new("bfm_vec_t*")
		assert not lib.bfm_vec_create(self.c_vec, default_state, len(vec))

		for i, val in enumerate(vec):
			self.c_vec.data[i] = val

	def __del__(self):
		assert not lib.bfm_vec_destroy(self.c_vec)
