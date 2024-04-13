#include <string.h>

#include <bfm/matrix.h>

#if defined(WITH_BLAS)
# include <cblas.h>
#endif

// full matrix

static int matrix_full_copy(bfm_matrix_t* matrix, bfm_matrix_t* src) {
	size_t const size = src->m * src->m * sizeof *matrix->full.data;
	memcpy(matrix->full.data, src->full.data, size);

	return 0;
}

static int matrix_full_destroy(bfm_matrix_t* matrix) {
	bfm_state_t* const state = matrix->state;

	state->free(matrix->full.data);
	return 0;
}

static double matrix_full_get(bfm_matrix_t* matrix, size_t i, size_t j) {
	if (i >= matrix->m || j >= matrix->m) {
		return BFM_NAN;
	}

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ? i * matrix->m + j : i + j * matrix->m;

	return matrix->full.data[idx];
}

static int matrix_full_set(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (i >= matrix->m || j >= matrix->m) {
		return -1;
	}

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ? i * matrix->m + j : i + j * matrix->m;

	matrix->full.data[idx] = val;
	return 0;
}

static int matrix_full_add(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (i >= matrix->m || j >= matrix->m) {
		return -1;
	}

	int const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ? i * matrix->m + j : i + j * matrix->m;

	matrix->full.data[idx] += val;
	return 0;
}

static size_t matrix_full_bandwidth(bfm_matrix_t* matrix) {
	size_t k = 0;

	for (ssize_t i = 0; i < (ssize_t) matrix->m; i++) {
		for (ssize_t j = 0; j < (ssize_t) matrix->m; j++) {
			if (!bfm_matrix_get(matrix, i, j)) {
				continue;
			}

			k = BFM_MAX((ssize_t) k, BFM_ABS(i - j));
		}
	}

	return k;
}

static int matrix_full_lu(bfm_matrix_t* matrix) {
	size_t const size = matrix->m;

	for (size_t pivot_i = 0; pivot_i < size - 1; pivot_i++) {
		// TODO handle error case and non square matrix

		double const pivot = matrix_full_get(matrix, pivot_i, pivot_i);

		if (pivot != pivot) {
			return -1;
		}

		if (fabs(pivot) < BFM_PIVOT_EPS) {
			return -1;
		}

		for (size_t i = pivot_i + 1; i < size; i++) {
			double row_val = matrix_full_get(matrix, i, pivot_i);

			if (BFM_IS_NAN(row_val)) {
				return -1;
			}

			double factor = row_val / pivot;

			// set pivot to one

			if (matrix_full_set(matrix, i, pivot_i, factor) < 0) {
				return -1;
			}

#if defined(WITH_BLAS)
			cblas_daxpy(size - pivot_i - 1, -factor, matrix->full.data + pivot_i * size + pivot_i + 1, 1, matrix->full.data + i * size + pivot_i + 1, 1);
#else
			for (size_t j = pivot_i + 1; j < size; j++) {
				double const val_row_pivot = matrix_full_get(matrix, pivot_i, j);

				// A[i][j] -= A[i][k] * A[k][j]

				if (matrix_full_add(matrix, i, j, -factor * val_row_pivot) < 0) {
					return -1;
				}
			}
#endif
		}
	}

	return 0;
}

static int matrix_full_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	double* const y = vec->data;
	double const m = matrix->m;

	// forward substitution Lx = y

#if defined(WITH_BLAS)
	CBLAS_LAYOUT const layout = matrix->major == BFM_MATRIX_MAJOR_ROW ? CblasRowMajor : CblasColMajor;

	cblas_dtrsv(layout, CblasLower, CblasNoTrans, CblasUnit, m, matrix->full.data, m, y, 1);
#else
	for (size_t i = 0; i < matrix->m; i++) {
		for (size_t j = 0; j < i; j++) {
			double const val = matrix_full_get(matrix, i, j);

			if (BFM_IS_NAN(val)) {
				return -1;
			}

			y[i] -= val * y[j];
		}
	}
#endif

	// backward substitution Ux = L^-1 @ y

#if defined(WITH_BLAS)
	cblas_dtrsv(layout, CblasUpper, CblasNoTrans, CblasNonUnit, m, matrix->full.data, m, y, 1);
#else
	for (ssize_t i = m - 1; i >= 0; i--) {
		for (size_t j = i + 1; j < matrix->m; j++) {
			double const val = matrix_full_get(matrix, i, j);

			if (BFM_IS_NAN(val)) {
				return -1;
			}

			y[i] -= val * y[j];
		}

		double const pivot = matrix_full_get(matrix, i, i);

		if (BFM_IS_NAN(pivot)) {
			return -1;
		}

		y[i] /= pivot;
	}
#endif

	return 0;
}

// band matrix routines

static int matrix_band_copy(bfm_matrix_t* matrix, bfm_matrix_t* src) {
	if (matrix->band.k != src->band.k) {
		return -1;
	}

	size_t const size = src->m * (src->band.k * 2 + 1) * sizeof *src->band.data;
	memcpy(matrix->band.data, src->band.data, size);

	return 0;
}

static int matrix_band_destroy(bfm_matrix_t* matrix) {
	bfm_state_t* const state = matrix->state;
	state->free(matrix->band.data);

	return 0;
}

static double matrix_band_get(bfm_matrix_t* matrix, size_t i, size_t j) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m) {
		return BFM_NAN;
	}

	if (BFM_ABS((ssize_t) i - (ssize_t) j) > (ssize_t) k) {
		return 0;
	}

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ? j + i * 2 * k : i + j * 2 * k;

	return matrix->band.data[idx];
}

static int matrix_band_set(bfm_matrix_t* matrix, size_t i, size_t j, double value) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m) {
		return -1;
	}

	if (BFM_ABS((ssize_t) i - (ssize_t) j) > (ssize_t) k) {
		return fabs(value) < BFM_PIVOT_EPS ? 0 : -1;
	}

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ? j + i * 2 * k : i + j * 2 * k;

	matrix->band.data[idx] = value;
	return 0;
}

static int matrix_band_add(bfm_matrix_t* matrix, size_t i, size_t j, double value) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	if (i >= m || j >= m) {
		return -1;
	}

	if (BFM_ABS((ssize_t) i - (ssize_t) j) > (ssize_t) k) {
		return fabs(value) < BFM_PIVOT_EPS ? 0 : -1;
	}

	size_t const idx = matrix->major == BFM_MATRIX_MAJOR_ROW ? j + i * 2 * k : i + j * 2 * k;

	matrix->band.data[idx] += value;
	return 0;
}

static size_t matrix_band_bandwidth(bfm_matrix_t* matrix) {
	return matrix->band.k;
}

static int matrix_band_lu(bfm_matrix_t* matrix) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	for (size_t pivot_i = 0; pivot_i < m - 1; pivot_i++) {
		double const pivot = matrix_band_get(matrix, pivot_i, pivot_i);

		if (BFM_IS_NAN(pivot)) {
			return -1;
		}

		if (fabs(pivot) < BFM_PIVOT_EPS) {
			return -1;
		}

		size_t const len = BFM_MIN(pivot_i + k + 1, m);

		for (size_t i = pivot_i + 1; i < len; i++) {
			double val_below_pivot = matrix_band_get(matrix, i, pivot_i);

			if (BFM_IS_NAN(val_below_pivot)) {
				return -1;
			}

			val_below_pivot /= pivot;

			if (matrix_band_set(matrix, i, pivot_i, val_below_pivot) < 0) {
				return -1;
			}

#if defined(WITH_BLAS)
			cblas_daxpy(len - pivot_i - 1, -val_below_pivot, matrix->band.data + pivot_i * (2 * k + 1) + pivot_i + 1, 1, matrix->band.data + i * (2 * k + 1) + pivot_i + 1, 1);
#else
			for (size_t j = pivot_i + 1; j < len; j++) {
				double const val = matrix_band_get(matrix, pivot_i, j);

				if (BFM_IS_NAN(val)) {
					return -1;
				}

				if (matrix_band_add(matrix, i, j, -val_below_pivot * val) < 0) {
					return -1;
				}
			}
#endif
		}
	}

	return 0;
}

__attribute__((unused)) static int matrix_band_cholesky(bfm_matrix_t* matrix) {
	size_t const m = matrix->m;

	for (size_t i = 0; i < m; i++) {
		size_t const len = BFM_MAX(0, matrix->band.k - i);

		for (size_t j = len; j < i; j++) {
			double s = 0;

			for (size_t k = len; k < j; k++) {
				double const a = matrix_band_get(matrix, i, k);
				double const b = matrix_band_get(matrix, j, k);

				if (BFM_IS_NAN(a) || BFM_IS_NAN(b)) {
					return -1;
				}

				s += a * b;
			}

			double const val = matrix_band_get(matrix, i, j);

			if (BFM_IS_NAN(val)) {
				return -1;
			}

			if (i == j && matrix_band_set(matrix, i, j, sqrt(val - s)) < 0) {
				return -1;
			}

			else {
				double const pivot = matrix_band_get(matrix, j, j);

				if (BFM_IS_NAN(val)) {
					return -1;
				}

				if (matrix_band_set(matrix, i, j, 1 / pivot * (val - s)) < 0) {
					return -1;
				}
			}
		}
	}

	return 0;
}

static int matrix_band_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	size_t const m = matrix->m;
	size_t const k = matrix->band.k;

	// forward substitution

	for (ssize_t pivot_i = 0; pivot_i < (ssize_t) m; pivot_i++) {
		ssize_t const len = BFM_MAX(pivot_i - (ssize_t) k, 0);

#if defined(WITH_BLAS)
		vec->data[pivot_i] -= cblas_ddot(pivot_i - len, matrix->band.data + pivot_i * (2 * k + 1) + len, 1, vec->data + len, 1);
#else
		for (ssize_t i = len; i < pivot_i; i++) {
			double const val = matrix_band_get(matrix, pivot_i, i);

			if (BFM_IS_NAN(val)) {
				return -1;
			}

			vec->data[pivot_i] -= val * vec->data[i];
		}
#endif
	}

	// backward substitution

	for (ssize_t pivot_i = m - 1; pivot_i >= 0; pivot_i--) {
		ssize_t const len = BFM_MIN(pivot_i + k + 1, m);

#if defined(WITH_BLAS)
		vec->data[pivot_i] -= cblas_ddot(len - pivot_i - 1, matrix->band.data + pivot_i * (2 * k + 1) + pivot_i + 1, 1, vec->data + pivot_i + 1, 1);
#else
		for (ssize_t i = pivot_i + 1; i < len; i++) {
			double const val = matrix_band_get(matrix, pivot_i, i);

			if (BFM_IS_NAN(val)) {
				return -1;
			}

			vec->data[pivot_i] -= vec->data[i] * val;
		}
#endif

		double const pivot = matrix_band_get(matrix, pivot_i, pivot_i);

		if (BFM_IS_NAN(pivot) || !pivot) {
			return -1;
		}

		vec->data[pivot_i] /= pivot;
	}

	return 0;
}

// generic matrix routines

int bfm_matrix_copy(bfm_matrix_t* matrix, bfm_matrix_t* src) {
	if (matrix->m != src->m) {
		return -1;
	}

	// bespoke copying functions
	// these are here to make copying faster than the generic method

	if (matrix->kind == BFM_MATRIX_KIND_FULL && src->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_copy(matrix, src);
	}

	if (matrix->kind == BFM_MATRIX_KIND_BAND && src->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_copy(matrix, src);
	}

	// generic method for copying matrices

	for (size_t i = 0; i < matrix->m; i++) {
		for (size_t j = 0; j < matrix->m; j++) {
			double const val = bfm_matrix_get(src, i, j);

			if (BFM_IS_NAN(val)) {
				return -1;
			}

			if (bfm_matrix_set(matrix, i, j, val) < 0) {
				return -1;
			}
		}
	}

	return 0;
}

int bfm_matrix_destroy(bfm_matrix_t* matrix) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_destroy(matrix);
	}

	if (matrix->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_destroy(matrix);
	}

	return -1;
}

double bfm_matrix_get(bfm_matrix_t* matrix, size_t i, size_t j) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_get(matrix, i, j);
	}

	if (matrix->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_get(matrix, i, j);
	}

	return -1;
}

int bfm_matrix_set(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_set(matrix, i, j, val);
	}

	if (matrix->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_set(matrix, i, j, val);
	}

	return -1;
}

int bfm_matrix_add(bfm_matrix_t* matrix, size_t i, size_t j, double val) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_add(matrix, i, j, val);
	}

	else if (matrix->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_add(matrix, i, j, val);
	}

	return -1;
}

size_t bfm_matrix_bandwidth(bfm_matrix_t* matrix) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_bandwidth(matrix);
	}

	else if (matrix->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_bandwidth(matrix);
	}

	return -1;
}

int bfm_matrix_lu(bfm_matrix_t* matrix) {
	if (matrix->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_lu(matrix);
	}

	if (matrix->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_lu(matrix);
	}

	return -1;
}

int bfm_matrix_lu_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	if (matrix->m != vec->n) {
		return -1;
	}

	if (matrix->kind == BFM_MATRIX_KIND_FULL) {
		return matrix_full_lu_solve(matrix, vec);
	}

	if (matrix->kind == BFM_MATRIX_KIND_BAND) {
		return matrix_band_lu_solve(matrix, vec);
	}

	return -1;
}

int bfm_matrix_solve(bfm_matrix_t* matrix, bfm_vec_t* vec) {
	if (bfm_matrix_lu(matrix) < 0) {
		return -1;
	}

	if (bfm_matrix_lu_solve(matrix, vec) < 0) {
		return -1;
	}

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

	if (matrix->full.data == NULL) {
		return -1;
	}

	memset(matrix->full.data, 0, size);

	return 0;
}

int bfm_matrix_band_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_major_t major, size_t m, size_t k) {
	matrix_create(matrix, state, BFM_MATRIX_KIND_BAND, major, m);
	matrix->band.k = k;

	size_t const size = m * (k * 2 + 1) * sizeof *matrix->band.data;
	matrix->band.data = state->alloc(size);

	if (matrix->band.data == NULL) {
		return -1;
	}

	memset(matrix->band.data, 0, size);

	return 0;
}
