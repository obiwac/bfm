#pragma once

#include <math.h>
#include "state.h"
#define PIVOT_EPS 1e-20

typedef enum {
	BFM_MATRIX_KIND_FULL,
	BFM_MATRIX_KIND_BAND,
} bfm_matrix_kind_t;

typedef enum {
	BFM_MATRIX_MAJOR_ROW,
	BFM_MATRIX_MAJOR_COLUMN,
} bfm_matrix_major_t;

typedef struct {
	size_t m;
	size_t n;

	bfm_matrix_major_t major;

	double* data; // don't access this directly; there are accessor functions for this
} bfm_matrix_full_t;

typedef struct {
	size_t m;
	size_t n;
	size_t k;

	double* data;
} bfm_matrix_band_t;

typedef struct {
	bfm_state_t* state;

	bfm_matrix_kind_t kind;
	bfm_matrix_major_t major;

	size_t m;
	size_t n;

	union {
		bfm_matrix_full_t full;
		bfm_matrix_band_t band;
	};
} bfm_matrix_t;

/**
 * @brief Allocate a matrix of size mxn
 * 
 * @param Pointer to a full matrix
 * @param m, number of rows
 * @param n, number of columns
 * @param major, the way the matrix is represented in memory
 * @return int, 0 if success, -1 if failure
 */
int bfm_matrix_full_allocate(bfm_matrix_full_t *full_matrix, size_t m, size_t n, bfm_matrix_major_t major);

/**
 * @brief Free the memory taken by full_matrix
 * 
 * @param Pointer to a full matrix
 * @return int, 0 if succes, -1 if failure
 */
int bfm_matrix_full_free(bfm_matrix_full_t *full_matrix);

/**
 * @brief Get value at index (i,j) in the matrix
 * 
 * @param Pointer to a full matrix
 * @param i, index of row
 * @param j, index of column
 * @return double, value stored at index (i,j)
 */
double bfm_matrix_full_get(bfm_matrix_full_t *full_matrix, int i, int j);

/**
 * @brief Set value at index (i,j) of the matrix
 * 
 * @param Pointer to a full matrix
 * @param i, index of row
 * @param j, index of column
 * @param value, to store at index (i,j)
 * @return int, 0 if succes, -1 if failure
 */
int bfm_matrix_full_set(bfm_matrix_full_t *full_matrix, int i, int j, double value);

int bfm_matrix_full_lu(bfm_matrix_full_t *full_matrix);

int bfm_matrix_full_lu_solve(bfm_matrix_full_t *full_matrix, double *y);

int bfm_matrix_full_solve(bfm_matrix_full_t *full_matrix, double *y);
