#include <string.h>

#include <bfm/rule.h>

int bfm_rule_create(bfm_rule_t* rule, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind, size_t n_points) {
	rule->state = state;

	rule->dim = dim;
	rule->kind = kind;
	rule->n_points = n_points;

	size_t const weights_size = n_points * sizeof *rule->weights;
	rule->weights = state->alloc(weights_size);

	if (rule->weights == NULL)
		goto err_weights;

	memset(rule->weights, 0, weights_size);

	size_t const points_size = dim * sizeof *rule->points;
	rule->points = state->alloc(points_size);

	if (rule->points == NULL)
		goto err_points;

	// zero out points so we can check for nullity to free on allocation error

	memset(rule->points, 0, points_size);

	for (size_t i = 0; i < dim; i++) {
		size_t const points_dim_size = n_points * sizeof **rule->points;
		double* const points_dim = state->alloc(points_dim_size);

		if (points_dim == NULL)
			goto err_points_dim;

		memset(points_dim, 0, points_dim_size);
		rule->points[i] = points_dim;
	}

	return 0;

err_points_dim:

	for (size_t i = 0; i < dim; i++) {
		double* const points_dim = rule->points[i];

		if (points_dim != NULL)
			state->free(points_dim);
	}

err_points:

	state->free(rule->weights);

err_weights:

	return -1;
}

int bfm_rule_destroy(bfm_rule_t* rule) {
	bfm_state_t* const state = rule->state;

	state->free(rule->weights);

	for (size_t i = 0; i < rule->dim; i++)
		state->free(rule->points[i]);

	state->free(rule->points);

	return 0;
}

// specific integration rule creation functions

static double const gauss_legendre_tri_weights[3] = { 0.166666666666667, 0.166666666666667, 0.166666666666667};
static double const gauss_legendre_simplex_xsi[3] = { 0.166666666666667, 0.666666666666667, 0.166666666666667};
static double const gauss_legendre_simplex_eta[3] = { 0.166666666666667, 0.166666666666667, 0.666666666666667};

static double const gauss_legendre_quad_weights[4] = { 1.000000000000000, 1.000000000000000, 1.000000000000000, 1.000000000000000};
static double const gauss_legendre_quad_xsi[4] = { -0.577350269189626, -0.577350269189626, 0.577350269189626, 0.577350269189626};
static double const gauss_legendre_quad_eta[4] = { 0.577350269189626, -0.577350269189626, -0.577350269189626, 0.577350269189626};

int bfm_rule_create_gauss_legendre(bfm_rule_t* rule, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind) {
	// only 2D Gauss-Legendre integration rules are supported and only on simplices or quads

	if (dim != 2)
		return -1;

	if (kind != BFM_ELEM_KIND_SIMPLEX && kind != BFM_ELEM_KIND_QUAD)
		return -1;

	// create generic integration rule object

	size_t const n_points = kind;

	if (bfm_rule_create(rule, state, dim, kind, n_points) < 0)
		return -1;

	// fill in weights & points

	if (kind == BFM_ELEM_KIND_SIMPLEX) {
		memcpy(rule->weights, gauss_legendre_tri_weights, sizeof gauss_legendre_tri_weights);

		memcpy(rule->points[0], gauss_legendre_simplex_xsi, sizeof gauss_legendre_simplex_xsi);
		memcpy(rule->points[1], gauss_legendre_simplex_eta, sizeof gauss_legendre_simplex_eta);
	}

	else if (kind == BFM_ELEM_KIND_QUAD) {
		memcpy(rule->weights, gauss_legendre_quad_weights, sizeof gauss_legendre_quad_weights);

		memcpy(rule->points[0], gauss_legendre_quad_xsi, sizeof gauss_legendre_quad_xsi);
		memcpy(rule->points[1], gauss_legendre_quad_eta, sizeof gauss_legendre_quad_eta);
	}

	return 0;
}
