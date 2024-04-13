#pragma once

#include <bfm/bfm.h>
#include <bfm/matrix.h>

typedef enum {
	BFM_FORCE_KIND_NONE,
	BFM_FORCE_KIND_LINEAR,
	BFM_FORCE_KIND_FUNKY,
} bfm_force_kind_t;

typedef struct {
	bfm_vec_t force;
} bfm_force_linear_t;

typedef struct bfm_force_t bfm_force_t; // forward declaration

typedef int (*bfm_force_funky_func_t)(bfm_force_t* force, bfm_vec_t* pos, bfm_vec_t* force_ref, void* data);

typedef struct {
	bfm_force_funky_func_t func;
	void* data;
} bfm_force_funky_t;

struct bfm_force_t {
	bfm_state_t* state;
	bfm_force_kind_t kind;
	size_t dim;

	union {
		bfm_force_linear_t linear;
		bfm_force_funky_t funky;
	};
};

int bfm_force_create(bfm_force_t* force, bfm_state_t* state, size_t dim);
int bfm_force_destroy(bfm_force_t* force);

int bfm_force_set_none(bfm_force_t* force);
int bfm_force_set_linear(bfm_force_t* force, bfm_vec_t* vec);

// TODO alternative bfm_force_set_funky_b if blocks are available

int bfm_force_set_funky(bfm_force_t* force, bfm_force_funky_func_t func, void* data);

int bfm_force_eval(bfm_force_t* force, bfm_vec_t* pos, bfm_vec_t* force_ref);
