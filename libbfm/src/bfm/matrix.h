#pragma once

#include <math.h>
#include <sys/types.h>
#include <stdbool.h>

#include <bfm/bfm.h>
#include <bfm/math.h>

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
	size_t k; // Size of the band
	double* data;
} bfm_matrix_band_t;

typedef struct {
	bfm_state_t* state;

	bfm_matrix_kind_t kind;
	bfm_matrix_major_t major; // TODO should this be called 'majority'?

	size_t m;
	// TODO property for symmetric matrices

	union {
		bfm_matrix_full_t full;
		bfm_matrix_band_t band;
	};
} bfm_matrix_t;

typedef struct {
	bfm_matrix_t* A;
	bfm_vec_t* B;
} bfm_system_t;

/**
 * @brief Create a full square matrix of size mxn
 *
 * @param matrix, pointer to matrix struct
 * @param state, pointer to state struct
 * @param major, the way the matrix is represented in memory (order)
 * @param m, number of rows/columns
 * @return int, 0 if success, -1 if failure
 */
int bfm_matrix_full_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_major_t major, size_t m);

/**
 * @brief Create a band square matrix of size mxn
 *
 * @param matrix, pointer to matrix struct
 * @param state, pointer to state struct
 * @param major, the way the matrix is represented in memory (order)
 * @param m, number of rows/columns
 * @param k, bandwidth of the matrix
 * @return int, 0 if success, -1 if failure
 */
int bfm_matrix_band_create(bfm_matrix_t* matrix, bfm_state_t* state, bfm_matrix_major_t major, size_t m, size_t k);

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

/**
 * @brief Apply LU decompition to a matrix; store it in place
 * 
 * @param A matrix 
 * @return int, 0 if succes, -1 if failure
 */
int bfm_matrix_lu(bfm_matrix_t* matrix);

/**
 * @brief solve a LUx = y system inplace
 * 
 * @param A LU matrix
 * @param y, a vector 
 * @return int, 0 if succes, -1 if failure 
 */
int bfm_matrix_lu_solve(bfm_matrix_t* matrix, bfm_vec_t *y);

/**
 * @brief solve a Ax = y system using lu decomposition
 * 
 * @param A, a matrix 
 * @param y
 * @return int 
 */
int bfm_matrix_solve(bfm_matrix_t* matrix, bfm_vec_t* y);
