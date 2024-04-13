#include <string.h>

#include <bfm/force.h>

int bfm_force_create(bfm_force_t* force, bfm_state_t* state, size_t dim) {
	memset(force, 0, sizeof *force);

	force->state = state;
	force->dim = dim;

	return 0;
}

int bfm_force_destroy(bfm_force_t* force) {
	(void) force;
	// TODO free linear vector
	return 0;
}

int bfm_force_set_none(bfm_force_t* force) {
	force->kind = BFM_FORCE_KIND_NONE;
	return 0;
}

int bfm_force_set_linear(bfm_force_t* force, bfm_vec_t* vec) {
	force->kind = BFM_FORCE_KIND_LINEAR;

	// check force vector dimension is correct

	if (vec->n != force->dim) {
		return -1;
	}

	if (bfm_vec_create(&force->linear.force, vec->state, vec->n) < 0) {
		return -1;
	}

	if (bfm_vec_copy(&force->linear.force, vec) < 0) {
		return -1;
	}

	return 0;
}

int bfm_force_set_funky(bfm_force_t* force, bfm_force_funky_func_t func, void* data) {
	force->kind = BFM_FORCE_KIND_FUNKY;

	force->funky.func = func;
	force->funky.data = data;

	return 0;
}

// force evaluation functions per kind

int eval_none(bfm_force_t* force, bfm_vec_t* pos, bfm_vec_t* force_ref) {
	(void) force;
	(void) pos;

	memset(force_ref->data, 0, force_ref->n * sizeof *force_ref->data);

	return 0;
}

int eval_linear(bfm_force_t* force, bfm_vec_t* pos, bfm_vec_t* force_ref) {
	(void) pos; // linear forces don't care about the position they're evaluated at

	if (bfm_vec_copy(force_ref, &force->linear.force) < 0) {
		return -1;
	}

	return -1;
}

int eval_funky(bfm_force_t* force, bfm_vec_t* pos, bfm_vec_t* force_ref) {
	return force->funky.func(force, pos, force_ref, force->funky.data);
}

int bfm_force_eval(bfm_force_t* force, bfm_vec_t* pos, bfm_vec_t* force_ref) {
	// check force vector dimension is correct

	if (force_ref->n != force->dim) {
		return -1;
	}

	if (force->kind == BFM_FORCE_KIND_NONE) {
		return eval_none(force, pos, force_ref);
	}

	if (force->kind == BFM_FORCE_KIND_LINEAR) {
		return eval_linear(force, pos, force_ref);
	}

	if (force->kind == BFM_FORCE_KIND_FUNKY) {
		return eval_funky(force, pos, force_ref);
	}

	return -1;
}
