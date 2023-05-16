#include <string.h>
#include <cblas.h>

#include <bfm/matrix.h>

// full matrix

static int matrix_full_destroy(bfm_matrix_t* matrix) {
	bfm_state_t* const state = matrix->state;

	state->free(matrix->full.data);
	return 0;
}

static double matrix_full_get(bfm_matrix_t* matrix, size_t i, size_t j) {
	if (i >= matrix->m || j >= matrix->m)
		return BFM_NAN;

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * matrix->m + j :
		i + j * matrix->m;

	return matrix->full.data[idx];
}

static int matrix_full_set(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (i >= matrix->m || j >= matrix->m)
		return -1;

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * matrix->m + j :
		i + j * matrix->m;

	matrix->full.data[idx] = val;
	return 0;
}

static int matrix_full_add(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (i >= matrix->m || j >= matrix->m)
		return -1;

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * matrix->m + j :
		i + j * matrix->m;

	matrix->full.data[idx] += val;
	return 0;
}

static int matrix_full_lu(bfm_matrix_t* matrix) {
	size_t const size = matrix->m;

	for (size_t pivot_i = 0; pivot_i < size - 1; pivot_i++) {
		// TODO handle error case and non square matrix

		double const pivot = matrix_full_get(matrix, pivot_i, pivot_i);

		if (pivot != pivot)
			return -1;
		
		if (fabs(pivot) < BFM_PIVOT_EPS)
			return -1;

		for (size_t i = pivot_i + 1; i < size; i++) {
			double row_val = matrix_full_get(matrix, i, pivot_i);

			if (BFM_IS_NAN(row_val))
				return -1;

			double factor = row_val / pivot;

			// set pivot to one

			if (matrix_full_set(matrix, i, pivot_i, factor) < 0)
				return -1;

			cblas_daxpy(size - pivot_i - 1, - factor, matrix->full.data + pivot_i * size + pivot_i + 1, 1, matrix->full.data + i * size + pivot_i + 1, 1);
			// for (size_t j = pivot_i + 1; j < size; j++) {
				// double const val_row_pivot = matrix_full_get(matrix, pivot_i, j);

				// A[i][j] -= A[i][k] * A[k][j]

				// if (matrix_full_add(matrix, i, j, - factor * val_row_pivot) < 0)
					// return -1;
			// }
		}
	}

	return 0;
}

static int matrix_full_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	double* const y = vec->data;
	double const m = matrix->m;
	CBLAS_LAYOUT layout = matrix->major == BFM_MATRIX_MAJOR_ROW ? CblasRowMajor : CblasColMajor;

	// forward substitution Lx = y
	cblas_dtrsv(layout, CblasLower, CblasNoTrans, CblasUnit, m, matrix->full.data, m, y, 1);

	// for (size_t i = 0; i < matrix->m; i++) {
	// 	for (size_t j = 0; j < i; j++) {
	// 		double const val = matrix_full_get(matrix, i, j);
	// 		y[i] -= val * y[j];
	// 	}
	// }
	// backward substitution Ux = L^-1 @ y
	cblas_dtrsv(layout, CblasUpper, CblasNoTrans, CblasNonUnit, m, matrix->full.data, m, y, 1);

	// for (ssize_t i = m - 1; i >= 0; i--) {
	// 	for (size_t j = i + 1; j < matrix->m; j++) {
	// 		double const val = matrix_full_get(matrix, i, j);
	// 		y[i] -= val * y[j];
	// 	}

	// 	y[i] /= matrix_full_get(matrix, i, i);
	// }

	return 0;
}

// band matrix routines

static int matrix_band_destroy(bfm_matrix_t *matrix) {
	bfm_state_t* const state = matrix->state;
	state->free(matrix->band.data);

	return 0;
}

static double matrix_band_get(bfm_matrix_t *matrix, size_t i, size_t j) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m)
		return BFM_NAN;

	if (j + 2 * k < i || j > i + 2 * k)
		return 0.;

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * (2 * k + 1) + j :
		i + j * (2 * k + 1);

	return matrix->band.data[idx];
}

static int matrix_band_set(bfm_matrix_t *matrix, size_t i, size_t j, double value) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m)
		return -1;

	if (j + 2 * k < i || j > i + 2 *k)
		return fabs(value) < BFM_PIVOT_EPS ? 0 : -1;

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * (2 * k + 1) + j :
		i + j * (2 * k + 1);

	matrix->band.data[idx] = value;
	return 0;
}

static int matrix_band_add(bfm_matrix_t *matrix, size_t i, size_t j, double value) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m)
		return -1;

	if (j + 2 * k < i || j > i + 2 * k)
		return fabs(value) < BFM_PIVOT_EPS ? 0 : -1;

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ?
		i * (2 * k + 1) + j :
		i + j * (2 * k + 1);

	matrix->band.data[idx] += value;
	return 0;
}

static int matrix_band_lu(bfm_matrix_t* matrix) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	for (size_t pivot_i = 0; pivot_i < m - 1; pivot_i++) {
		double const pivot = matrix_band_get(matrix, pivot_i, pivot_i);

		if (pivot != pivot)
			return -1;

		if (fabs(pivot) < BFM_PIVOT_EPS)
			return -1;

		size_t const max_i = BFM_MIN(pivot + k + 1, m);

		for (size_t i = pivot_i + 1; i < max_i; i++) {
			double val_below_pivot = matrix_band_get(matrix, i, pivot_i);

			if (BFM_IS_NAN(val_below_pivot))
				return -1;

			double factor = val_below_pivot / pivot;

			if (matrix_band_set(matrix, i, pivot_i, factor) < 0)
				return -1;

			cblas_daxpy(max_i - pivot_i - 1, - factor, matrix->band.data + pivot_i * (2 * k + 1) + pivot_i + 1, 1, matrix->band.data + i * (2 * k + 1) + pivot_i + 1, 1);
			// for (size_t j = pivot_i + 1; j < max_i; j++) {
				// double const val = matrix_band_get(matrix, pivot_i, j);

				// if (BFM_IS_NAN(val))
					// return -1;

				// if (matrix_band_add(matrix, i, j, - factor * val) < 0)
					// return -1;
			// }
		}
	}

	return 0;
}

static int matrix_band_cholesky(bfm_matrix_t* matrix) {
	for (size_t i = 0; i < matrix->m; i++) {
		size_t min_j = BFM_MAX(0, matrix->band.k - i);
		for (size_t j = min_j; j < i; j++) {
			double s = 0;
			for (size_t k = min_j; k < j; k++) {
				s += matrix_band_get(matrix, i, k) * matrix_band_get(matrix, j, k);
			}

			double val = matrix_band_get(matrix, i, j);
			if (i == j)
				matrix_band_set(matrix, i, j, sqrt(val - s));
			else {
				double pivot = matrix_band_get(matrix, j, j);
				matrix_band_set(matrix, i, j, 1 / pivot * (val - s));
			}
		}
	}
	return 0;
}

static int matrix_band_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	for (size_t pivot_i = 0; pivot_i < m; pivot_i++) {
		ssize_t diff = pivot_i - k;
		size_t const min_i = BFM_MAX(diff, 0);
		vec->data[pivot_i] -= cblas_ddot(pivot_i - min_i, matrix->band.data + pivot_i * (2 * k + 1) + min_i, 1, vec->data + min_i, 1);
		// for (size_t j = min_i; j < pivot_i; j++) {
			// double const val = matrix_band_get(matrix, pivot_i, j);
			// vec->data[pivot_i] -= val * vec->data[j];
		// }
	}

	for (ssize_t pivot_i = m - 1; pivot_i >= 0; pivot_i--) {
		size_t const max_i = BFM_MIN(pivot_i + k + 1, m);
		vec->data[pivot_i] -= cblas_ddot(max_i - pivot_i - 1, matrix->band.data + pivot_i * (2 * k + 1) + pivot_i + 1, 1, vec->data + pivot_i + 1, 1);
		// for (size_t j = pivot_i + 1; j < max_i; j++) {
			// double const val = matrix_band_get(matrix, pivot_i, j);

			// if (BFM_IS_NAN(val))
				// return -1;

			// vec->data[pivot_i] -= vec->data[j] * val;
		// }

		double const pivot = matrix_band_get(matrix, pivot_i, pivot_i);

		if (BFM_IS_NAN(pivot))
			return -1;

		vec->data[pivot_i] /= pivot;
	}

	return 0;
}

// generic matrix routines

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

int bfm_matrix_add(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL)
		return matrix_full_add(matrix, i, j, val);

	else if (matrix->kind == BFM_MATRIX_KIND_BAND)
		return matrix_band_add(matrix, i, j, val);

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

// creation functions

static int matrix_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_kind_t kind, bfm_matrix_major_t major, size_t m) {
	matrix->state = state;

	matrix->kind = kind;
	matrix->major = major;
	matrix->m = m;

	return -1;
}

int bfm_matrix_full_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_major_t major, size_t m) {
	matrix_create(matrix, state, BFM_MATRIX_KIND_FULL, major, m);

	size_t const size = m * m * sizeof *matrix->full.data;
	matrix->full.data = state->alloc(size);

	if (matrix->full.data == NULL)
		return -1;

	memset(matrix->full.data, 0, size);

	return 0;
}

int bfm_matrix_band_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_major_t major, size_t m, size_t k) {
	matrix_create(matrix, state, BFM_MATRIX_KIND_BAND, major, m);
	matrix->band.k = k;

	size_t const size = m * (k * 2 + 1) * sizeof *matrix->band.data;
	matrix->band.data = state->alloc(size);

	if (matrix->band.data == NULL)
		return -1;

	memset(matrix->band.data, 0, size);

	return 0;
}
