#pragma once

#include <bfm/mesh.h>

typedef enum {
	BFM_CONDITION_KIND_DIRICHLET_X = 0,
	BFM_CONDITION_KIND_DIRICHLET_Y = 1,
	BFM_CONDITION_KIND_NEUMANN_X = 2,
	BFM_CONDITION_KIND_NEUMANN_Y = 3,
	BFM_CONDITION_KIND_NEUMANN_NORMAL = 4,
	BFM_CONDITION_KIND_NEUMANN_TANGENT = 5,
	BFM_CONDITION_KIND_DIRICHLET_NORMAL = 6,
	BFM_CONDITION_KIND_DIRICHLET_TANGENT = 7,
} bfm_condition_kind_t;

typedef struct {
	bfm_state_t* state;
	bfm_mesh_t* mesh;

	bfm_condition_kind_t kind;

	double value;

	// for each node of the mesh, "true" indicates it's part of this boundary condition

	bool* nodes;
} bfm_condition_t;

int bfm_condition_create(bfm_condition_t* condition, bfm_state_t* state, bfm_mesh_t* mesh, bfm_condition_kind_t kind);
int bfm_condition_destroy(bfm_condition_t* condition);

// TODO bfm_condition_populate to add nodes to boundary condition based on a passed function
//      if blocks are available, there should be a bfm_condition_populate_b variant
