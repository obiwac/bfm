#include <stdio.h>
#include <string.h>

#include <bfm/bfm.h>

int bfm_state_create(bfm_state_t* state) {
	memset(state, 0, sizeof *state);

	state->alloc = malloc;
	state->free = free;

	return 0;
}

int bfm_state_destroy(bfm_state_t* state) {
	(void) state;
	return 0;
}

int bfm_set_alloc(bfm_state_t* state, bfm_alloc_t alloc) {
	state->alloc = alloc;
	return 0;
}

int bfm_set_free(bfm_state_t* state, bfm_free_t free) {
	state->free = free;
	return 0;
}

int bfm_err_print(bfm_state_t* state) {
	bfm_err_t* const err = &state->err;

	if (!err->has)
		return 0;

	printf("[BFM %s:%zu (%s)] %s\n", err->file, err->line, err->func, err->msg);

	return 0;
}
