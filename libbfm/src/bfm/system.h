#pragma once

#include <bfm/force.h>
#include <bfm/instance.h>
#include <bfm/math.h>
#include <bfm/matrix.h>
#include <bfm/perm.h>

typedef struct {
	bfm_state_t* state;

	size_t n;

	bfm_perm_t perm;
	bfm_matrix_t A;
	bfm_vec_t b;
} bfm_system_t;

int bfm_system_create(bfm_system_t* system, bfm_state_t* state, size_t n);
int bfm_system_destroy(bfm_system_t* system);

int bfm_system_renumber(bfm_system_t* system);

// system creation functions per kind

int bfm_system_create_elasticity(bfm_system_t* system, bfm_instance_t* instance, size_t n_forces, bfm_force_t** forces);
