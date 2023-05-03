#pragma once

#include <bfm/mesh.h>

typedef struct {
	bfm_state_t* state;
	bfm_mesh_t* mesh;

	// for each node of the mesh, "true" indicates it's part of this boundary condition

	bool* nodes;
} bfm_condition_t;

int bfm_condition_create(bfm_condition_t* condition, bfm_state_t* state, bfm_mesh_t* mesh);
int bfm_condition_destroy(bfm_condition_t* condition);

// TODO bfm_condition_populate to add nodes to boundary condition based on a passed function
//      if blocks are available, there should be a bfm_condition_populate_b variant
