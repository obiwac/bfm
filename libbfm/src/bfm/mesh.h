#pragma once

#include <stdlib.h>

#include <bfm/matrix.h>

typedef enum {
	BFM_ELEM_KIND_SIMPLEX = 3,
	BFM_ELEM_KIND_QUAD = 4,
} bfm_elem_kind_t;

typedef enum {
	BFM_PLANAR_STRAINS,
} bfm_problem_type_t;

typedef struct {
	size_t n; // Number of points of integrations
	double const* weights;
	double const* xsi;
	double const* eta;
} bfm_rule_t;

typedef struct {
	double const* xsi;
	double const* eta;
	
	int (*get_phi) (double xsi, double eta, double* phi);
	
	int (*get_dphidxsi) (double xsi, double eta, double* dphidxsi);
	int (*get_dphideta) (double xsi, double eta, double* dphideta);
} bfm_shape_functions_t;

typedef struct {
	// src (nodes[0]) <-> dst (nodes[1])
	// maybe rename src and dst ?
	size_t nodes[2];
	// the two faces that are adjacent to the edges, elem[1] is -1 if the edges is on the boundary
	size_t elems[2];
} bfm_edge_t;

typedef struct {
	bfm_state_t* state;

	size_t dim;
	bfm_elem_kind_t kind;

	size_t n_elems;
	size_t n_nodes;
	size_t n_edges;

	double* coords;
	size_t* elems;
	bfm_edge_t* edges;
	// bool* boundary_nodes;

	// TODO move rule from mesh to ??
	bfm_rule_t rule;
	// Should the shape function be stored in the mesh ?
	bfm_shape_functions_t functions;
} bfm_mesh_t;

typedef struct {
	size_t n_local_nodes;
	// Store only pointers ? and allocate ? 
	size_t map[4];
	double x[4];
	double y[4];
} bfm_local_element_t;

int bfm_mesh_create_generic(bfm_mesh_t* mesh, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind);
int bfm_mesh_destroy(bfm_mesh_t* mesh);

int bfm_mesh_read_lepl1110(bfm_mesh_t* mesh, bfm_state_t* state, char const* name);
