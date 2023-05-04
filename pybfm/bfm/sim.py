from .force import Force
from .instance import Instance
from .libbfm import lib, ffi
from .state import default_state

class Sim:
	DEFORMATION = 0

	def __init__(self, kind: int):
		self.c_sim = ffi.new("bfm_sim_t*")
		assert not lib.bfm_sim_create(self.c_sim, default_state, kind)

	def __del__(self):
		assert not lib.bfm_sim_destroy(self.c_sim)

	def add_instance(self, instance: Instance):
		assert not lib.bfm_sim_add_instance(self.c_sim, instance.c_instance)

	def add_force(self, force: Force):
		assert not lib.bfm_sim_add_force(self.c_sim, force.c_force)

	def run(self):
		assert not lib.bfm_sim_run(self.c_sim)
