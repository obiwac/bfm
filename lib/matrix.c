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

	for (ssize_t i = m - 1; i >= 0; i--) {
		for (size_t j = i + 1; j < matrix->m; j++) {
			double const val = matrix_full_get(matrix, i, j);
			y[i] -= val * y[j];
		}
	}

	return 0;
}

// band matrix routines

static int matrix_band_create(bfm_matrix_t* matrix, size_t k) {
	bfm_state_t* const state = matrix->state;
	matrix->band.k = k;
	// matrix->band.symmetric = symmetric;

	size_t const size = matrix->m * (k * 2 + 1) * sizeof *matrix->band.data;
	matrix->band.data = state->alloc(size);
}

static int matrix_band_destroy(bfm_matrix_t *matrix) {
	bfm_state_t* const state = matrix->state;
	state->free(matrix->band.data);
}

static double matrix_band_get(bfm_matrix_t *matrix, size_t i, size_t j) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m)
		return 0.0 / 0.0; // NaN

	if (j + k < i|| j > i + k)
		return 0.;

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * k + j :
		i + j * k;
	return matrix->band.data[idx];
}

static int matrix_band_set(bfm_matrix_t *matrix, size_t i, size_t j, double value) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m)
		return -1;

	if (j + k < i || j > i + k)
		return fabs(value) < BFM_PIVOT_EPS ? 0 : -1;

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * k + j :
		i + j * k;
	matrix->band.data[idx] = value;
	return 0;
}

static int matrix_band_lu(bfm_matrix_t* matrix) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;
	int err;

	for (size_t pivot = 0; pivot < m - 1; ++pivot) {
		double pivot_value = matrix_band_get(matrix, pivot, pivot);
		// TODO DEAL WITH NAN
		if (fabs(pivot) < BFM_PIVOT_EPS)
			return -1;

		size_t max_i = pivot + k + 1;
		if (max_i > m) // Could be replaced by max macro
			max_i = m;

		for (int i = pivot + 1; i < max_i; i++) {
			double value_below_pivot = matrix_band_get(matrix, i, pivot);
			value_below_pivot /= pivot;
			err = matrix_band_set(matrix, i, pivot, value_below_pivot);
			if (err != 0)
				return -1;

			for (int j = pivot + 1; j < max_i; j++) {
				// TODO handle nan value
				double const value = matrix_band_get(matrix, i, j);
				double const value_upside = matrix_band_get(matrix, pivot, j);
				err = matrix_band_set(matrix, i, j, value - value_upside * value_below_pivot);
				if (err != 0)
					return -1;
			}
		}
	}
	return 0;
}

static int matrix_band_lu_solve(bfm_matrix_t *matrix, bfm_vec_t *vec) {
	size_t m = matrix->m;
	size_t k = matrix->band.k;

	for (size_t pivot = 0; pivot < m; ++pivot) {
		ssize_t min_i = pivot - k;
		if (min_i < 0)
			min_i = 0; // COuld be replaced by min macro
		
		for (size_t i = min_i; i < pivot; ++i) {
			double value = matrix_band_get(matrix, pivot, i);
			vec->data[pivot] -= value * vec->data[i];
		}
	}

	for (ssize_t pivot = m - 1; pivot >= 0; --pivot) {
		size_t mx = pivot + k + 1;
		if (mx > m)
			mx = m;
		for (int i = pivot + 1; i < mx; i++) {
			double value = matrix_band_get(matrix, pivot, i);
			// TODO handle nan case
			vec->data[k] -= vec->data[i] * value;
		}
		double pivot_value = matrix_band_get(matrix, pivot, pivot);
		// TODO handle nan case
		vec->data[k] /= pivot_value;
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

	// if (matrix->kind == BFM_MATRIX_KIND_BAND)
		// return matrix_band_create(matrix);

	return -1;
}

int bfm_matrix_destroy(bfm_matrix_t* matrix) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_destroy(matrix);

	if (matrix->kind == BFM_MATRIX_KIND_BAND)
		return matrix_band_destroy(matrix);

	return -1;
}

double bfm_matrix_get(bfm_matrix_t* matrix, size_t i, size_t j) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_get(matrix, i, j);

	if (matrix->kind == BFM_MATRIX_KIND_BAND)
		return matrix_band_get(matrix, i, j);

	return -1;
}

int bfm_matrix_set(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_set(matrix, i, j, val);

	if (matrix->kind == BFM_MATRIX_KIND_BAND)
		return matrix_band_set(matrix, i, j, val);

	return -1;
}

int bfm_matrix_lu(bfm_matrix_t* matrix) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_lu(matrix);

	if (matrix->kind == BFM_MATRIX_KIND_BAND)
		return matrix_band_lu(matrix);

	return -1;
}

int bfm_matrix_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	if (matrix->m != vec->n)
		return -1;

	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_lu_solve(matrix, vec);

	if (matrix->kind == BFM_MATRIX_KIND_BAND)
		return matrix_band_lu_solve(matrix, vec);

	return -1;
}

int bfm_matrix_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	if (bfm_matrix_lu(matrix) < 0)
		return -1;

	if (bfm_matrix_lu_solve(matrix, vec) < 0)
		return -1;

	return 0;
}
