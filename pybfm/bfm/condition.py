from .libbfm import lib, ffi
from .mesh import Mesh
from .state import default_state

from collections.abc import Callable

class Condition:
	DIRICHLET_X       = 0
	DIRICHLET_Y       = 1
	NEUMANN_X         = 2
	NEUMANN_Y         = 3
	NEUMANN_NORMAL    = 4
	NEUMANN_TANGENT   = 5
	DIRICHLET_NORMAL  = 6
	DIRICHLET_TANGENT = 7

	def __init__(self, mesh: Mesh, kind: int):
		self.c_condition = ffi.new("bfm_condition_t*")
		assert not lib.bfm_condition_create(self.c_condition, default_state, mesh.c_mesh, kind)

		self.mesh = mesh

	def __del__(self):
		assert not lib.bfm_condition_destroy(self.c_condition)

	def populate(self, discriminator: Callable[[Mesh, tuple[float]], bool]):
		for i in range(self.mesh.c_mesh.n_nodes):
			coord = tuple(self.mesh.coords[i * self.mesh.dim + j] for j in range(self.mesh.dim))
			self.c_condition.nodes[i] = discriminator(self.mesh, coord)
