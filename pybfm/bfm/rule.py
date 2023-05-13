from .libbfm import lib, ffi
from .state import default_state

class Rule:
	def __init__(self, dim: int, kind: int, n_points: int):
		self.c_rule = ffi.new("bfm_rule_t*")
		assert not lib.bfm_rule_create(self.c_rule, default_state, dim, kind, n_points)

	def __del__(self):
		assert not lib.bfm_rule_destroy(self.c_rule)

class Rule_gauss_legendre(Rule):
	def __init__(self, dim: int, kind: int):
		self.c_rule = ffi.new("bfm_rule_t*")
		assert not lib.bfm_rule_create_gauss_legendre(self.c_rule, default_state, dim, kind)
