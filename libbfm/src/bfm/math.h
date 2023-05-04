#pragma once

#define BFM_NAN (0. / 0.)
#define BFM_IS_NAN(x) ((x) != (x))
#define BFM_PIVOT_EPS 1e-20 // XXX

#define BFM_MAX(a, b) ((a) > (b) ? (a) : (b))
#define BFM_MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
	bfm_state_t* state;

	size_t n;
	double* data;
} bfm_vec_t;
