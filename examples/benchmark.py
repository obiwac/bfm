import time

from bfm import Bfm, Mesh_lepl1110, Mesh_wavefront, Ez_lepl1110

bfm = Bfm()

mesh = Mesh_lepl1110("meshes/8.lepl1110")
ez = Ez_lepl1110(mesh, "problems/problem.txt")

avg = 0
total = 10
for _ in range(total):
    start = time.time()
    ez.sim.run()
    avg += (time.time() - start) / total

print(f"Average time : {avg}s")
