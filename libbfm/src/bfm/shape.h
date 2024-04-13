#pragma once

#include <bfm/mesh.h>

typedef struct bfm_shape_t bfm_shape_t;

// bfm_shape_fn_t -> shape function
// bfm_shape_dfn_t -> derivative of shape function wrt a component

typedef int (*bfm_shape_fn_t)(bfm_shape_t* shape, double* point, double* phi);
typedef int (*bfm_shape_dfn_t)(bfm_shape_t* shape, size_t wrt, double* point, double* dphi);

struct bfm_shape_t {
	bfm_state_t* state;

	size_t dim;
	bfm_elem_kind_t kind;

	bfm_shape_fn_t phi;
	bfm_shape_dfn_t dphi;
};

int bfm_shape_create(bfm_shape_t* shape, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind);
int bfm_shape_destroy(bfm_shape_t* shape);
