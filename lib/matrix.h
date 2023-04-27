#pragma once

typedef enum {
	BFM_MATRIX_KIND_FULL,
	BFM_MATRIX_KIND_BAND,
} bfm_matrix_kind_t;

typedef enum {
	BFM_MATRIX_MAJOR_ROW,
	BFM_MATRIX_MAJOR_COLUMN,
} bfm_matrix_major_t;

typedef struct {
	double* data; // don't access this directly; there are accessor functions for this
} bfm_matrix_full_t;

typedef struct {
	bfm_state_t* state;

	bfm_matrix_kind_t kind;
	bfm_matrix_major_t major;

	size_t n;
	size_t m;

	union {
		bfm_matrix_full_t full;
		bfm_matrix_band_t band;
	};
} bfm_matrix_t;
