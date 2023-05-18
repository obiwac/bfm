from .force import Force
from .instance import Instance
from .libbfm import lib, ffi
from .shader import Shader
from .state import default_state

class Sim:
	DEFORMATION = 0

	def __init__(self, kind: int):
		self.c_sim = ffi.new("bfm_sim_t*")
		assert not lib.bfm_sim_create(self.c_sim, default_state, kind)

		self.instances: list[Instance] = []

		# create shader, depending on simulation kind
		# TODO in fine, make these package resources

		if kind == Sim.DEFORMATION:
			self.shader = Shader("shaders/sim/deformation.vert", "shaders/sim/deformation.frag")

	def __del__(self):
		assert not lib.bfm_sim_destroy(self.c_sim)

	def add_instance(self, instance: Instance):
		assert not lib.bfm_sim_add_instance(self.c_sim, instance.c_instance)
		self.instances.append(instance)

	def add_force(self, force: Force):
		assert not lib.bfm_sim_add_force(self.c_sim, force.c_force)

	def run(self):
		assert not lib.bfm_sim_run(self.c_sim)
	
	@classmethod
	def read_lepl1110(cls, mesh, name):
		sim = cls(Sim.DEFORMATION)
		c_str = ffi.new("char[]", bytes(name, "utf-8"))
		assert not lib.bfm_sim_read_lepl1110(sim.c_sim, mesh.c_mesh, default_state, c_str)
		return sim

	# visualisation functions

	def show(self):
		for instance in self.instances:
			instance.update_effects()

	def draw(self):
		self.shader.use()

		for instance in self.instances:
			instance.draw()
