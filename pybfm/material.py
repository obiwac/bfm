class Material:
	AA7075 = Material("AA7075", 2.81e3, 71.7e9, 0.33, (0.667, 0.439, 0.459))

	def __init__(self, name: str, rho: float, E: float, nu: float, colour: tuple[float] = (1, 1, 1)):
		self.name = name

		self.rho = rho # density
		self.E = E # Young's modulus (at room temperature)
		self.nu = nu # Poisson's ratio

		self.colour = colour
