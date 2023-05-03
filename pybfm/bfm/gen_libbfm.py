import re
import ctypes

from cffi import FFI

# generate python bindings

if __name__ == "__main__":
	ffi = FFI()

	headers = [
		"bfm/bfm.h",
		"bfm/material.h",
		"bfm/math.h",
		"bfm/matrix.h",
		"bfm/mesh.h",
		"bfm/condition.h",
		"bfm/obj.h",
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
		libraries=["bfm"],
		include_dirs=["/usr/local/include"],
	)

	ffi.compile(verbose=True)
