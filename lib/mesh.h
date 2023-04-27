#pragma once

#include <stdlib.h>

typedef enum {
	BFM_QUADS,
	BFM_TRIANGLES
} bfm_element_type_t;

typedef struct {
	size_t n_elems;
	size_t n_local_nodes;
	bfm_element_type_t type;

	size_t *elem;
	double *coords;
} bfm_mesh_t;


typedef struct {
	size_t n_local_nodes;
	bfm_element_type_t type;

	size_t *nodes;
	double *coords;
} bfm_local_element;

