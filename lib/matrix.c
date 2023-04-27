#include "matrix.h"

int bfm_matrix_full_allocate(bfm_matrix_full_t *full_matrix, size_t m, size_t n, bfm_matrix_major_t major) {
    full_matrix->m = m;
    full_matrix->n = n;
    full_matrix->major = major;
    full_matrix->data = calloc(n * m, sizeof(double));

    if (full_matrix->data == NULL)
        return -1;
    return 0;
}

int bfm_matrix_full_free(bfm_matrix_full_t *full_matrix) {
    free(full_matrix->data);
    return 0;
}

double bfm_matrix_full_get(bfm_matrix_full_t *full_matrix, int i, int j) {
    // TODO handle out of bound;
    int idx;
    if (full_matrix->major == BFM_MATRIX_MAJOR_ROW)
        idx = i * full_matrix->n + j;
    else
        idx = i + j * full_matrix->m;
    return full_matrix->data[idx];
}

int bfm_matrix_full_set(bfm_matrix_full_t *full_matrix, int i, int j, double value) {
    if (0 > i || i >= full_matrix->n || 0 > j || j <= full_matrix->m)
        return -1;

    int idx;
    if (full_matrix->major == BFM_MATRIX_MAJOR_ROW)
        idx = i * full_matrix->n + j;
    else
        idx = i + full_matrix->m + j;
    full_matrix->data[idx] = value;
    return 0;
}

int bfm_matrix_full_lu(bfm_matrix_full_t *full_matrix) {
    size_t m = full_matrix->m;
    size_t n = full_matrix->n;

    int err;

    for (int k = 0; k < m - 1; k++) {
        // TODO Handle error case and non square matrix
        double pivot_value = bfm_matrix_full_get(full_matrix, k, k);
        if (fabs(pivot_value) < PIVOT_EPS)
            return -1;
        
        for (int i = k + 1; i < m; i++) {
            double row_value = bfm_matrix_full_get(full_matrix, i, k);
            row_value /= pivot_value;
            // Set pivot to one
            err = bfm_matrix_full_set(full_matrix, i, k, row_value);
            if (err != 0)
                return err;
            for (int j = k + 1; j < n; j++) {
                double value = bfm_matrix_full_get(full_matrix, i, j);
                double value_row_pivot = bfm_matrix_full_get(full_matrix, k, j);
                // A[i][j] -= A[i][k] * A[k][j]
                err = bfm_matrix_full_set(full_matrix, i, j, value - row_value * value_row_pivot);
                if (err != 0)
                    return err;
            }
        }
    }
    return 0;
}

int bfm_matrix_full_lu_solve(bfm_matrix_full_t *full_matrix, double *y) {
    size_t m = full_matrix->m;
    
    // Forward substitution Lx = y 
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < i; j++) {
            double value = bfm_matrix_full_get(full_matrix, i, j);
            y[i] -= value * y[j];
        }
    }

    // Backward substitution Ux = L^-1 y
    for (int i = m - 1; i >= 0; i--) {
        for (int j = i + 1; j < m; j++) {
            double value = bfm_matrix_full_get(full_matrix, i, j);
            y[i] -= value * y[j];
        }
    }
    return 0;
}

int bfm_matrix_full_solve(bfm_matrix_full_t *full_matrix, double *y) {
    bfm_matrix_full_lu(full_matrix);
    bfm_matrix_full_lu_solve(full_matrix, y);
}

int bfm_matrix_band_allocate(bfm_matrix_band_t *band_matrix, int m, int k) {
    band_matrix->k = k;
    band_matrix->m = m;
    band_matrix->data = calloc(m * k, sizeof(double));
    if (band_matrix->data == NULL)
        return -1;
    return 0;
}

int bfm_matrix_band_free(bfm_matrix_band_t *band_matrix) {
    free(band_matrix->data);
}
