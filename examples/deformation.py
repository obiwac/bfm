import faulthandler
faulthandler.enable()

import math

from bfm import Bfm, Condition, Force_linear, Instance, Mesh_lepl1110, Mesh_wavefront, Material, Obj, Rule_gauss_legendre, Sim

# create initial BFM context
# TODO should this be renamed something a little clearer, e.g. Scene?

print("Starting BFM")

bfm = Bfm()

bfm.set_default_recoil(1.7000000000000006)
bfm.set_default_rotation([-0.42500000000000016, -0.4450000000000002])
bfm.set_default_origin([-1.134999999999998, -0.9550000000000005, 0])

# create a 2D simplex mesh
# this is simply a horizontal bar 1 unit wide and 0.2 units tall

print("Loading meshes")

# mesh = Mesh(2, Mesh.SIMPLEX)
#
# mesh.rect((0, 0), (1, 0.2))
# mesh.mesh()

terrain = Mesh_wavefront("meshes/terrain.obj", True)
bfm.add_scenery(terrain)

# mesh = Mesh_lepl1110("meshes/8.lepl1110")
mesh = Mesh_wavefront("meshes/bridge-dam.obj")

# create Dirichlet boundary conditions for mesh
# add all nodes close to the centre

cross = []

def cache_cross():
	print("Caching cross-section")

	for i in range(terrain.c_mesh.n_nodes):
		tz = terrain.c_mesh.coords[i * 3 + 2]

		if abs(tz) > 0.04:
			continue

		tx = terrain.c_mesh.coords[i * 3 + 0]
		ty = terrain.c_mesh.coords[i * 3 + 1]

		cross.append((tx, ty))

	with open("meshes/cross.py", "w") as f:
		f.write(f"{cross}")

def read_cross():
	global cross

	with open("meshes/cross.py", "r") as f:
		cross = eval(f.read())

# cache_cross()
read_cross()

def is_boundary(mesh, coord):
	x, y = coord
	z = 0 # all coordinates are gonna be aligned at z = 0

	smallest_diff = (999, 0)

	for tx, ty in cross:
		diff = abs(tx - x)

		if diff < smallest_diff[0]:
			smallest_diff = (diff, ty)

	return y < smallest_diff[1]

print("Computing boundary conditions")

boundary_condition_x = Condition(mesh, Condition.DIRICHLET_X)
boundary_condition_x.populate(is_boundary)

boundary_condition_y = Condition(mesh, Condition.DIRICHLET_Y)
boundary_condition_y.populate(is_boundary)

# create object out of 7075-series aluminium:
# density (rho): 2.81 g/cm^3
# Young's modulus (E): 71.7 GPa
# Poisson's ratio (nu): 0.33
# there are other properties such as tensile strength we could consider but this is enough for now
# for the material used in the LEPL1110 course: Material("Steel", rho=7.85e3, E=211.e9, nu=0.3)

material = Material.AA7075
rule = Rule_gauss_legendre(mesh.dim, mesh.kind)
obj = Obj(mesh, material, rule)

# create instance from object and boundary conditions
# add the instance to the state

instance = Instance(obj)

instance.add_condition(boundary_condition_x)
instance.add_condition(boundary_condition_y)

# create basic gravity force field

gravity = Force_linear.EARTH_GRAVITY_2D

# create simulation
# run the simulation

print("Run simulation")

sim = Sim(Sim.PLANAR_STRESS)

sim.add_instance(instance)
sim.add_force(gravity)

sim.run()

# display results
# resulting effects from the simulation will automatically be applied to the instance we added to our scene previously

bfm.show(sim)
