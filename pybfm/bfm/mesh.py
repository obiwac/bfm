class Mesh:
	SIMPLEX = 0
	QUAD = 1

	def __init__(self, dim: int, kind: int):
		self.dim = dim
		self.kind = kind

	def rect(self, first: tuple[float], second: tuple[float], cut: bool = False):
		...

	def mesh(self):
		...
