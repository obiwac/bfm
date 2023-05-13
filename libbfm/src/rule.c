#include <string.h>

#include <bfm/rule.h>

int bfm_rule_create(bfm_rule_t* rule, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind, size_t n_points) {
	memset(rule, 0, sizeof *rule);
	rule->state = state;

	rule->dim = dim;
	rule->kind = kind;
	rule->n_points = n_points;

	size_t const weights_size = n_points * sizeof *rule->weights;
	rule->weights = state->alloc(weights_size);

	if (rule->weights == NULL)
		goto err_weights;

	memset(rule->weights, 0, weights_size);

	size_t const points_size = n_points * sizeof *rule->points;
	rule->points = state->alloc(points_size);

	if (rule->points == NULL)
		goto err_points;

	// zero out points so we can check for nullity to free on allocation error

	memset(rule->points, 0, points_size);

	for (size_t i = 0; i < n_points; i++) {
		size_t const coords_size = dim * sizeof **rule->points;
		double* const coords = state->alloc(coords_size);

		if (coords == NULL)
			goto err_coords;

		memset(coords, 0, coords_size);
		rule->points[i] = coords;
	}

	return 0;

err_coords:

	for (size_t i = 0; i < dim; i++) {
		double* const coords = rule->points[i];

		if (coords != NULL)
			state->free(coords);
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

	// shape functions

	if (rule->phi)
		state->free(rule->phi);

	return 0;
}

int bfm_rule_populate_shape_funcs(bfm_rule_t* rule) {
	// XXX for now, only 3/4 point serendipity shape functions are supported

	if (rule->kind == BFM_ELEM_KIND_SIMPLEX) {
	}

	return 0;
}

// specific integration rule creation functions

#define THIRD (1. / 3)
#define SIXTH (1. / 6)
#define SQRT_THIRD 0.577350269189626

static double const gauss_legendre_simplex_weights[3] = { SIXTH, SIXTH, SIXTH };

static double const gauss_legendre_simplex_points[3][2] = {
	{     SIXTH,     SIXTH },
	{ 1 - THIRD,     SIXTH },
	{     SIXTH, 1 - THIRD },
};

static double const gauss_legendre_quad_weights[4] = { 1, 1, 1, 1 };

static double const gauss_legendre_quad_points[4][2] = {
	{ -SQRT_THIRD,  SQRT_THIRD },
	{ -SQRT_THIRD, -SQRT_THIRD },
	{  SQRT_THIRD, -SQRT_THIRD },
	{  SQRT_THIRD,  SQRT_THIRD },
};

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
		memcpy(rule->weights, gauss_legendre_simplex_weights, sizeof gauss_legendre_simplex_weights);

		for (size_t i = 0; i < sizeof(gauss_legendre_simplex_points) / sizeof(*gauss_legendre_simplex_points); i++)
			memcpy(rule->points[i], gauss_legendre_simplex_points[i], sizeof gauss_legendre_simplex_points[i]);
	}

	else if (kind == BFM_ELEM_KIND_QUAD) {
		memcpy(rule->weights, gauss_legendre_quad_weights, sizeof gauss_legendre_quad_weights);

		for (size_t i = 0; i < sizeof(gauss_legendre_quad_points) / sizeof(*gauss_legendre_quad_points); i++)
			memcpy(rule->points[i], gauss_legendre_quad_points[i], sizeof gauss_legendre_quad_points[i]);
	}

	return 0;
}
