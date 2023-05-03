from .libbfm import lib, ffi

default_state = ffi.new("bfm_state_t*")
lib.bfm_state_create(default_state)

# XXX to be correct, we'd have to call lib.bfm_state_destroy at the end
#     but I don't care and it doesn't matter
