from bfm import Bfm, Mesh_lepl1110, Mesh_wavefront, Ez_lepl1110

bfm = Bfm()

mesh = Mesh_lepl1110("meshes/8.lepl1110")
ez = Ez_lepl1110(mesh, "problems/problem.txt")

ez.sim.run()
ez.write("V.txt", 0)
ez.write("U.txt", 1)

bfm.show(ez.sim)
