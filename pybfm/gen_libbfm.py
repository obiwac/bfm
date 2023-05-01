import re
import ctypes

from cffi import FFI

# generate python bindings

if __name__ == "__main__":
	ffi = FFI()

	with open("libbfm/src/bfm/bfm.h") as f:
		src = f.read()

	src = re.sub("#include.*", "", src)
	ffi.cdef(src)

	ffi.set_source("pybfm.libbfm.libbfm", "#include \"../../libbfm/src/bfm/bfm.h\"", libraries=["bfm"])
	ffi.compile(verbose=True)
