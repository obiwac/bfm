#pragma once

#include <stdlib.h>

#include <bfm/matrix.h>

typedef enum {
	BFM_QUADS,
	BFM_TRIANGLES,
} bfm_element_type_t;

typedef enum {
	BFM_PLANAR_STRAINS,
} bfm_problem_type_t;

typedef struct {
	double* weights;
	double* xsi;
	double* eta;
	double* dphi_dxsi;
	double* dphi_deta;
} bfm_rule_t;

typedef struct {
	bfm_state_t* state;

	size_t n_elems;
	size_t n_nodes;
	size_t n_local_nodes;
	bfm_element_type_t type;

	bfm_rule_t* rule;
	size_t* elems;
	double* coords;
} bfm_mesh_t;

typedef struct {
	size_t n_local_nodes;
	size_t* map;
	double* x;
	double* y;
} bfm_local_element_t;

int bfm_build_elasticity_system(bfm_mesh_t* mesh, bfm_system_t* system, double young_modulus, double poisson_ratio, double rho, double force);
