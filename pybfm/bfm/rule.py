from .libbfm import lib, ffi
from .state import default_state

class CRule:
	def __init__(self, c_rule):
		self.c_rule = c_rule

class Rule(CRule):
	def __init__(self, dim: int, kind: int, n_points: int):
		c_rule = ffi.new("bfm_rule_t*")
		assert not lib.bfm_rule_create(c_rule, default_state, dim, kind, n_points)

		super().__init__(c_rule)

	def __del__(self):
		assert not lib.bfm_rule_destroy(self.c_rule)

class Rule_gauss_legendre(CRule):
	def __init__(self, dim: int, kind: int):
		c_rule = ffi.new("bfm_rule_t*")
		assert not lib.bfm_rule_create_gauss_legendre(c_rule, default_state, dim, kind)

		super().__init__(c_rule)

	def __del__(self):
		assert not lib.bfm_rule_destroy(self.c_rule)
