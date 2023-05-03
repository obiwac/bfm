from .material import Material
from .mesh import Mesh

class Obj:
	def __init__(self, mesh: Mesh, material: Material):
		self.mesh = mesh
		self.material = material

	def draw(self):
		self.mesh.draw()
