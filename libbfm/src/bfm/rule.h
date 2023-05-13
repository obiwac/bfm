#pragma once

#include <bfm/mesh.h>
#include <bfm/shape.h>

typedef struct {
	bfm_state_t* state;

	size_t dim;
	bfm_elem_kind_t kind;
	size_t n_points;

	double* weights;
	double** points; // XXX this isn't an array of points, but an array of coordinates

	bfm_shape_t shape;
} bfm_rule_t;

int bfm_rule_create(bfm_rule_t* rule, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind, size_t n_points);
int bfm_rule_destroy(bfm_rule_t* rule);

// specific integration rule creation functions

int bfm_rule_create_gauss_legendre(bfm_rule_t* rule, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind);
