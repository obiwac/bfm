from bfm import Bfm, Mesh_lepl1110, Mesh_wavefront, Ez_lepl1110

bfm = Bfm()

mesh = Mesh_lepl1110("meshes/8.lepl1110")
ez = Ez_lepl1110(mesh, "problems/problem.txt")

ez.sim.run()
bfm.show(ez.sim)
