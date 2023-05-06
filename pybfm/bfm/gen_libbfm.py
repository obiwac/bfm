import re
import ctypes

from cffi import FFI

# generate python bindings

if __name__ == "__main__":
	ffi = FFI()

	headers = [
		"bfm/bfm.h",
		"bfm/math.h",
		"bfm/force.h",
		"bfm/material.h",
		"bfm/matrix.h",
		"bfm/mesh.h",
		"bfm/condition.h",
		"bfm/obj.h",
		"bfm/instance.h",
		"bfm/sim.h",
		"bfm/elasticity.h"
	]

	includes = ""

	for header in headers:
		includes += f"#include <{header}>\n"

		with open(f"libbfm/src/{header}") as f:
			src = f.read()

			src = re.sub("#include.*", "", src)
			src = re.sub("#define.*", "", src)

			ffi.cdef(src)

	ffi.set_source(
		"pybfm.bfm.libbfm",
		includes,
		library_dirs=["/usr/local/lib"],
		include_dirs=["/usr/local/include"],
		libraries=["bfm"],
	)

	ffi.compile(verbose=True)
