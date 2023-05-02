from pybfm import Bfm, Mesh, Obj

# create initial BFM context

bfm = Bfm()

# create a 2D simplex mesh
# this is simply a horizontal bar 1 unit wide and 0.2 units tall

mesh = Mesh(2, Mesh.SIMPLEX)

mesh.rect((0, 0), (1, 0.2))
mesh.mesh()

# create object out of 7075-series aluminium:
# density (rho): 2.81 g/cm^3
# Young's modulus (E): 71.7 GPa
# Poisson's ratio (nu): 0.33
# there are other properties such as tensile strength we could consider but this is enough for now
# for the material used in the LEPL1110 course: Material("Steel", rho=7.85e3, E=211.e9, nu=0.3)

material = Material.AA7075
obj = Obj(mesh, material)
bfm.add(obj)

# TODO create setup from object and force field (default field types, like gravity field?)
# TODO add Dirichlet boundary condition to problem (with anonymous function to discriminate?)
# TODO create elasticity matrix and force vector from integration rule
# TODO solve problem proper, maybe this could be animated element-by-element so it isn't boring to wait?

# display results

bfm.show()
