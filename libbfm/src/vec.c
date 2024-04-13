#include <string.h>

#include <bfm/math.h>

int bfm_vec_create(bfm_vec_t* vec, bfm_state_t* state, size_t n) {
	vec->state = state;
	vec->n = n;

	size_t const size = n * sizeof *vec->data;
	vec->data = state->alloc(size);

	if (vec->data == NULL) {
		return -1;
	}

	memset(vec->data, 0, size);

	return 0;
}

int bfm_vec_copy(bfm_vec_t* vec, bfm_vec_t* src) {
	memcpy(vec->data, src->data, src->n * sizeof *vec->data);
	return 0;
}

int bfm_vec_destroy(bfm_vec_t* vec) {
	bfm_state_t* const state = vec->state;

	state->free(vec->data);

	return 0;
}
