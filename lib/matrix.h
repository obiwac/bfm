#pragma once

#include <math.h>
#include "state.h"

#define BFM_PIVOT_EPS 1e-20

typedef enum {
	BFM_MATRIX_KIND_FULL,
	BFM_MATRIX_KIND_BAND,
} bfm_matrix_kind_t;

typedef enum {
	BFM_MATRIX_MAJOR_ROW,
	BFM_MATRIX_MAJOR_COLUMN,
} bfm_matrix_major_t;

typedef struct {
	double* data;
} bfm_matrix_full_t;

typedef struct {
	double* data;
} bfm_matrix_band_t;

typedef struct {
	bfm_state_t* state;

	bfm_matrix_kind_t kind;
	bfm_matrix_major_t major; // TODO should this be called 'majority'?

	size_t m;

	union {
		bfm_matrix_full_t full;
		bfm_matrix_band_t band;
	};
} bfm_matrix_t;

typedef struct {
	size_t n;
	double *data;
} bfm_vec_t;

/**
 * @brief Create a matrix of size mxn
 * 
 * @param matrix, pointer to matrix struct
 * @param state, pointer to state struct
 * @param kind, the matrix representation kind
 * @param major, the way the matrix is represented in memory (order)
 * @param m, number of rows
 * @param n, number of columns
 * @return int, 0 if success, -1 if failure
 */
int bfm_matrix_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_kind_t kind, bfm_matrix_major_t major, size_t m, size_t n);

/**
 * @brief Destroy a matrix
 * 
 * @param matrix, pointer to matrix struct
 * @return int, 0 if succes, -1 if failure
 */
int bfm_matrix_destroy(bfm_matrix_t* matrix);

/**
 * @brief Get value at index (i,j) in the matrix
 * 
 * @param matrix, pointer to matrix struct
 * @param i, index of row
 * @param j, index of column
 * @return double, value stored at index (i,j), NaN if failure
 */
double bfm_matrix_get(bfm_matrix_t* matrix, size_t i, size_t j);

/**
 * @brief Set value at index (i,j) of the matrix
 * 
 * @param matrix, pointer to matrix struct
 * @param i, index of row
 * @param j, index of column
 * @param value, to store at index (i,j)
 * @return int, 0 if succes, -1 if failure
 */
int bfm_matrix_set(bfm_matrix_t* matrix, size_t i, size_t j, double val);

int bfm_matrix_lu(bfm_matrix_t* matrix);

int bfm_matrix_lu_solve(bfm_matrix_t* matrix, bfm_vec_t *y);

int bfm_matrix_solve(bfm_matrix_t* matrix, bfm_vec_t *y);
