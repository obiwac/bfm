#include "matrix.h"

// full matrix

static int matrix_full_create(bfm_matrix_t* matrix) {
	bfm_state_t* const state = matrix->state;

	size_t const size = matrix->m * matrix->m * sizeof *matrix->full.data;
	matrix->full.data = state->alloc(size);

	if (matrix->full.data == NULL)
		return -1;

	memset(matrix->full.data, 0, size);
	return 0;
}

static int matrix_full_destroy(bfm_matrix_t* matrix) {
	bfm_state_t* const state = matrix->state;

	state->free(matrix->full.data);
	return 0;
}

static double matrix_full_get(bfm_matrix_t* matrix, size_t i, size_t j) {
	if (i < 0 || i >= matrix->m || j < 0 || j <= matrix->m)
		return 0.0 / 0.0; // NaN

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * matrix->m + j :
		i + j * matrix->m;

	return matrix->full.data[idx];
}

static int matrix_full_set(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (i < 0 || i >= matrix->m || j < 0 || j <= matrix->m)
		return -1;

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * matrix->m + j :
		i + j * matrix->m;

	matrix->full.data[idx] = val;
	return 0;
}

static int matrix_full_lu(bfm_matrix_t* matrix) {
	size_t const m = matrix->m;

	for (size_t k = 0; k < m - 1; k++) {
		// TODO handle error case and non square matrix

		double const pivot = matrix_full_get(matrix, k, k);

		if (fabs(pivot) < BFM_PIVOT_EPS)
			return -1;

		for (size_t i = k + 1; i < m; i++) {
			double row_val = matrix_full_get(matrix, i, k);
			row_val /= pivot;

			// set pivot to one
			if (matrix_full_set(matrix, i, k, row_val) < 0)
				return -1;

			for (size_t j = k + 1; j < m; j++) {
				double const val = matrix_full_get(matrix, i, j);
				double const val_row_pivot = matrix_full_get(matrix, k, j);

				// A[i][j] -= A[i][k] * A[k][j]

				if (matrix_full_set(matrix, i, j, val - row_val * val_row_pivot) < 0)
					return -1;
			}
		}
	}

	return 0;
}

static int matrix_full_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	double* const y = vec->data;
	double const m = matrix->m;

	// forward substitution Lx = y

	for (size_t i = 0; i < matrix->m; i++) {
		for (size_t j = 0; j < i; j++) {
			double const val = matrix_full_get(matrix, i, j);
			y[i] -= val * y[j];
		}
	}

	// backward substitution Ux = L^-1 y

	for (size_t i = m - 1; i >= 0; i--) {
		for (size_t j = i + 1; j < matrix->m; j++) {
			double const val = matrix_full_get(matrix, i, j);
			y[i] -= val * y[j];
		}
	}

	return 0;
}

// generic matrix routines

int bfm_matrix_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_kind_t kind, bfm_matrix_major_t major, size_t m, size_t n) {
	matrix->state = state;

	matrix->kind = kind;
	matrix->major = major;

	matrix->m = m;

	if (kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_create(matrix);

	// if (kind == BFM_MATRIX_KIND_BAND)
	// 	return matrix_band_create(matrix);

	return -1;
}

int bfm_matrix_destroy(bfm_matrix_t* matrix) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_destroy(matrix);

	// if (matrix->kind == BFM_MATRIX_KIND_BAND)
	// 	return matrix_band_destroy(matrix);

	return -1;
}

double bfm_matrix_get(bfm_matrix_t* matrix, size_t i, size_t j) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_get(matrix, i, j);

	// if (kind == BFM_MATRIX_KIND_BAND)
	// 	return matrix_band_get(matrix, i, j);

	return -1;
}

int bfm_matrix_set(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_set(matrix, i, j, val);

	// if (kind == BFM_MATRIX_KIND_BAND)
	// 	return matrix_band_set(matrix, i, j, val);

	return -1;
}

int bfm_matrix_lu(bfm_matrix_t* matrix) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_lu(matrix);

	// if (kind == BFM_MATRIX_KIND_BAND)
	// 	return matrix_band_lu(matrix);

	return -1;
}

int bfm_matrix_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	if (matrix->m != vec->n)
		return -1;

	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_lu_solve(matrix, vec);

	// if (kind == BFM_MATRIX_KIND_BAND)
	// 	return matrix_band_lu_solve(matrix);

	return -1;
}

int bfm_matrix_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	if (bfm_matrix_lu(matrix) < 0)
		return -1;

	if (bfm_matrix_lu_solve(matrix, vec) < 0)
		return -1;

	return 0;
}
