#pragma once

#include <bfm/bfm.h>
#include <bfm/mesh.h>

typedef int (*bfm_shape_fn_t) (size_t n, double* point, double* phi);

typedef struct {
	bfm_state_t* state;

	bfm_elem_kind_t kind;
	size_t dim;
	size_t n_points;

	double* weights;
	double** points; // XXX this isn't an array of points, but an array of coordinates

	// shape function and its derivatives wrt each point

	bfm_shape_fn_t phi;
} bfm_rule_t;

int bfm_rule_create(bfm_rule_t* rule, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind, size_t n_points);
int bfm_rule_destroy(bfm_rule_t* rule);

// specific integration rule creation functions

int bfm_rule_create_gauss_legendre(bfm_rule_t* rule, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind);
