from .libbfm import lib, ffi
from .condition import Condition
from .obj import Obj
from .state import default_state

class Instance:
	def __init__(self, obj: Obj):
		self.c_instance = ffi.new("bfm_instance_t*")
		assert not lib.bfm_instance_create(self.c_instance, default_state, obj.c_obj)

		self.obj = obj

	def __del__(self):
		assert not lib.bfm_instance_destroy(self.c_instance)

	def add_condition(self, condition: Condition)
		assert not lib.bfm_instance_add_condition(self.c_instance, condition.c_condition
