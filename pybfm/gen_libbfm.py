import re
import ctypes

from cffi import FFI

# generate python bindings

if __name__ == "__main__":
	ffi = FFI()

	includes = """
		#include <bfm/bfm.h>
		#include <bfm/math.h>
		#include <bfm/matrix.h>
		#include <bfm/mesh.h>
	"""

	ffi.set_source(
		"pybfm.libbfm.libbfm",
		includes,
		libraries=["bfm"],
		extra_compile_args=["-I/usr/local/include"]
	)

	ffi.compile(verbose=True)
