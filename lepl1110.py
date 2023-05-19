import sys

from bfm import Bfm, Mesh_lepl1110, Mesh_wavefront, Ez_lepl1110


bfm = Bfm()

mesh = sys.argv[1]
problem = sys.argv[2]

mesh = Mesh_lepl1110(mesh)
ez = Ez_lepl1110(mesh, problem)

ez.sim.run()
ez.write("U.txt", 0)
ez.write("V.txt", 1)

bfm.show(ez.sim)
