from bfm import Bfm, Condition, Force_linear, Instance, Mesh_lepl1110, Mesh_wavefront, Material, Obj, Rule_gauss_legendre, Sim

bfm = Bfm()

mesh = Mesh_wavefront("meshes/test.obj")

sim = Sim.read_lepl1110(mesh, "problems/problem.txt")

sim.run()

bfm.show(sim)