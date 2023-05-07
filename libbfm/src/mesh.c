#include <stdio.h>
#include <string.h>

#include <bfm/mesh.h>

// From lepl1110 fem.c
static double const _gaussQuad4Xsi[4]    = {-0.577350269189626,-0.577350269189626, 0.577350269189626, 0.577350269189626};
static double const _gaussQuad4Eta[4]    = { 0.577350269189626,-0.577350269189626,-0.577350269189626, 0.577350269189626};
static double const _gaussQuad4Weight[4] = { 1.000000000000000, 1.000000000000000, 1.000000000000000, 1.000000000000000};
static double const _gaussTri3Xsi[3]     = { 0.166666666666667, 0.666666666666667, 0.166666666666667};
static double const _gaussTri3Eta[3]     = { 0.166666666666667, 0.166666666666667, 0.666666666666667};
static double const _gaussTri3Weight[3]  = { 0.166666666666667, 0.166666666666667, 0.166666666666667};

static double const xsi_triangles[3] = {0., 1., 0.};
static double const eta_triangles[3] = {0., 0., 1.};
static double const xsi_quads[4] = {1., -1., -1., 1.};
static double const eta_quads[4] = {1., 1., -1., -1.};

int bfm_mesh_create_generic(bfm_mesh_t* mesh, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind) {
	memset(mesh, 0, sizeof *mesh);
	mesh->state = state;

	mesh->dim = dim;
	mesh->kind = kind;

	return 0;
}

int bfm_mesh_destroy(bfm_mesh_t* mesh) {
	bfm_state_t* const state = mesh->state;

	state->free(mesh->coords);
	state->free(mesh->elems);
	state->free(mesh->edges);

	return 0;
}

static int cmp_edge(const void* e1, const void* e2) {
	bfm_edge_t* edge1 = (bfm_edge_t*) e1;
	bfm_edge_t* edge2 = (bfm_edge_t*) e2;
	// Can I use unsigned int for m1, m2 with substraction afterwards?
	int m1 = BFM_MIN(edge1->nodes[0], edge1->nodes[1]);
	int m2 = BFM_MIN(edge2->nodes[0], edge2->nodes[1]);
	int diff = m1 - m2;
	if (diff > 0)
		return -1;
	if (diff < 0)
		return 1;

	int M1 = BFM_MAX(edge1->nodes[0], edge1->nodes[1]);
	int M2 = BFM_MAX(edge2->nodes[0], edge2->nodes[1]);

	return M1 - M2;
}

static int compute_edges(bfm_mesh_t* mesh) {
	size_t n_elems = mesh->n_elems;
	size_t n_local_nodes = mesh->kind;
	size_t n_edges = n_elems * n_local_nodes;

	mesh->edges = mesh->state->alloc(n_elems * n_local_nodes * sizeof(bfm_edge_t));
	if (!mesh->edges)
		return -1;

	for (size_t elem = 0; elem < n_elems; elem++) {
		for (size_t j = 0; j < n_local_nodes; j++) {
			mesh->edges[elem * n_local_nodes + j].nodes[0] = mesh->elems[elem * n_local_nodes + j];
			mesh->edges[elem * n_local_nodes + j].nodes[1] = mesh->elems[elem * n_local_nodes + (j + 1) % n_local_nodes];
			mesh->edges[elem * n_local_nodes + j].elems[0] = elem;
            mesh->edges[elem * n_local_nodes + j].elems[1] = -1;
		}
	}

	qsort(mesh->edges, n_edges, sizeof *mesh->edges, cmp_edge);


	size_t current = 0;
	for (size_t i = 1; i < n_edges; i++) {
		if ((mesh->edges[i - 1].nodes[0] == mesh->edges[i].nodes[1] && mesh->edges[i - 1].nodes[1] == mesh->edges[i].nodes[0])) {
			mesh->edges[current] = mesh->edges[i - 1];
			mesh->edges[current].elems[1] = mesh->edges[i].elems[0];
			i++;
		}
		else {
			mesh->edges[current] = mesh->edges[i - 1];
		}
		current++;
	}
	mesh->n_edges = current;
	mesh->edges = mesh->state->realloc(mesh->edges, current * sizeof(bfm_edge_t));
	if (!mesh->edges)
		return -1;

	// mesh->boundary_nodes = mesh->state->alloc(mesh->n_nodes * sizeof *mesh->boundary_nodes);
	// if (!mesh->boundary_nodes)
		// return -1;
	// memset(mesh->boundary_nodes, 0, mesh->n_nodes);
	// for (size_t i = 0; i < current; i++) {
	// 	if (mesh->edges[i].elems[1] == -1) {
	// 		mesh->boundary_nodes[mesh->edges[i].nodes[0]] = true;
	// 		mesh->boundary_nodes[mesh->edges[i].nodes[1]] = true;
	// 	}
	// }
	return 0;
}

int create_rule(bfm_rule_t* rule, bfm_elem_kind_t kind) {
    if (kind == BFM_ELEM_KIND_SIMPLEX) {
        rule->eta = _gaussTri3Eta;
		rule->xsi = _gaussTri3Xsi;
		rule->weights = _gaussTri3Weight;
    }
    else if (kind == BFM_ELEM_KIND_QUAD) {
		rule->eta = _gaussQuad4Eta;
		rule->xsi = _gaussQuad4Xsi;
		rule->weights = _gaussQuad4Weight;
    }
    else
        return -1;
    return 0;
}

static int triangle_phi(double xsi, double eta, double* phi) {
	phi[0] = 1 - xsi - eta;
	phi[1] = xsi;
	phi[2] = eta;
	return 0;
}

static int quads_phi(double xsi, double eta, double* phi) {
	phi[0] = (1. + xsi) * (1 + eta) / 4.;
	phi[1] = (1. - xsi) * (1 + eta) / 4.;
	phi[2] = (1. - xsi) * (1 - eta) / 4.;
	phi[3] = (1. + xsi) * (1 - eta) / 4.;
	return 0;
}

static int triangle_dphideta(double xsi, double eta, double* dphideta) {
	dphideta[0] = -1.;
	dphideta[1] = 0.;
	dphideta[2] = 1.;
	return 0;
}

static int triangle_dphidxsi(double xsi, double eta, double* dphidxsi) {
	dphidxsi[0] = -1.;
	dphidxsi[1] = 1.;
	dphidxsi[2] = 0.;
	return 0;
}

static int quads_dphideta(double xsi, double eta, double* dphideta) {
	dphideta[0] = (1. + xsi) / 4.;
	dphideta[1] = (1. - xsi) / 4.;
	dphideta[2] = - (1. - xsi) / 4.;
	dphideta[3] = - (1. + xsi) / 4.;
	return 0;
}

static int quads_dphidxsi(double xsi, double eta, double* dphidxsi) {
	dphidxsi[0] = (1. + eta) / 4.;
	dphidxsi[1] = - (1. + eta) / 4.;
	dphidxsi[2] = - (1. - eta) / 4.;
	dphidxsi[3] = (1. - eta) / 4.;
	return 0;
}

static int create_shape_functions(bfm_shape_functions_t* functions, bfm_elem_kind_t kind) {
	if (kind == BFM_ELEM_KIND_SIMPLEX) {
		functions->xsi = xsi_triangles;
		functions->eta = eta_triangles;
		
		functions->get_phi = triangle_phi;
		
		functions->get_dphideta = triangle_dphideta;
		functions->get_dphidxsi = triangle_dphidxsi;
	}
	else if (kind == BFM_ELEM_KIND_QUAD) {
		functions->xsi = xsi_quads;
		functions->eta = eta_quads;
		
		functions->get_phi = quads_phi;
		
		functions->get_dphideta = quads_dphideta;
		functions->get_dphidxsi = quads_dphidxsi;
	}
	else
		return -1;
	return 0;
	
}

int bfm_mesh_read_lepl1110(bfm_mesh_t* mesh, bfm_state_t* state, char const* name) {
	int rv = -1;

	mesh->state = state;
	mesh->dim = 2; // LEPL1110 only looks at 2D meshes

	// TODO error messages & more error checking (alloc's/fscanf's)

	FILE* const fp = fopen(name, "r");

	if (fp == NULL)
		goto err_fopen; // TODO error message

	// read nodes

	fscanf(fp, "Number of nodes %zu\n", &mesh->n_nodes);
	mesh->coords = state->alloc(mesh->n_nodes * 2 * sizeof *mesh->coords);

	size_t _;

	for (size_t i = 0; i < mesh->n_nodes; i++)
		fscanf(fp, "\t%zu :\t%lf\t%lf\n", &_, &mesh->coords[i * 2], &mesh->coords[i * 2 + 1]);

	// read elements

	char kind_str[16];
	fscanf(fp, "Number of %15s %zu\n", kind_str, &mesh->n_elems);

	if (strcmp(kind_str, "triangles") == 0)
		mesh->kind = BFM_ELEM_KIND_SIMPLEX;

	else if (strcmp(kind_str, "quads") == 0)
		mesh->kind = BFM_ELEM_KIND_QUAD;

	else
		goto err_kind; // TODO error message

	mesh->elems = state->alloc(mesh->n_elems * mesh->kind * sizeof *mesh->elems);

	for (size_t i = 0; mesh->kind == BFM_ELEM_KIND_SIMPLEX && i < mesh->n_elems; i++)
		fscanf(fp, "\t%zu :\t%zu\t%zu\t%zu\n", &_, &mesh->elems[i * 3], &mesh->elems[i * 3 + 1], &mesh->elems[i * 3 + 2]);

	for (size_t i = 0; mesh->kind == BFM_ELEM_KIND_QUAD && i < mesh->n_elems; i++)
		fscanf(fp, "\t%zu :\t%zu\t%zu\t%zu\t%zu\n", &_, &mesh->elems[i * 4], &mesh->elems[i * 4 + 1], &mesh->elems[i * 4 + 2], &mesh->elems[i * 4 + 3]);

	int err = compute_edges(mesh);
	if (err != 0) {
		free(mesh->elems);
		goto err_kind;
	}

	create_rule(&mesh->rule, mesh->kind);
	create_shape_functions(&mesh->functions, mesh->kind);
	// success
	rv = 0;

err_kind:

	fclose(fp);

err_fopen:

	return rv;
}
