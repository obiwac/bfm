#pragma once

#include <bfm/math.h>
#include <bfm/matrix.h>

typedef struct {
	bfm_state_t* state;

	size_t m;
	bool has_perm;

	size_t* perm;
	size_t* inv_perm;
} bfm_perm_t;

int bfm_perm_create(bfm_perm_t* perm, bfm_state_t* state, size_t m);
int bfm_perm_destroy(bfm_perm_t* perm);

int bfm_perm_perm_matrix(bfm_perm_t* perm, bfm_matrix_t* matrix, bool inv);
int bfm_perm_perm_vec(bfm_perm_t* perm, bfm_vec_t* vec, bool inv);

int bfm_perm_rcm(bfm_perm_t* perm, bfm_matrix_t* mat);
