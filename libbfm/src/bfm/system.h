#pragma once

#include <bfm/force.h>
#include <bfm/instance.h>
#include <bfm/math.h>
#include <bfm/matrix.h>

typedef enum {
	BFM_SYSTEM_KIND_GENERIC = 0,
	BFM_SYSTEM_KIND_ELASTICITY = 1,
} bfm_system_kind_t;

typedef struct {
	bfm_state_t* state;

	bfm_system_kind_t kind;
	size_t n;

	bfm_matrix_t A;
	bfm_vec_t b;
} bfm_system_t;

int bfm_system_create(bfm_system_t* system, bfm_state_t* state, size_t n);
int bfm_system_destroy(bfm_system_t* system);

// system creation functions per kind

int bfm_system_create_elasticity(bfm_system_t* system, bfm_instance_t* instance, size_t n_forces, bfm_force_t** forces);
