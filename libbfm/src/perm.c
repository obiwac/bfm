#include <string.h>

#include <bfm/perm.h>

int bfm_perm_create(bfm_perm_t* perm, bfm_state_t* state, size_t m) {
	memset(perm, 0, sizeof *perm);
	perm->state = state;

	perm->m = m;
	perm->has_perm = false;

	return 0;
}

int bfm_perm_destroy(bfm_perm_t* perm) {
	bfm_state_t* const state = perm->state;

	if (perm->has_perm) {
		state->free(perm->perm);
		state->free(perm->inv_perm);
	}

	return 0;
}

int bfm_perm_perm_matrix(bfm_perm_t* perm, bfm_matrix_t* matrix, bool inv) {
	bfm_state_t* const state = perm->state;

	if (!perm->has_perm)
		return -1;

	size_t* const cur_perm = inv ? perm->inv_perm : perm->perm;

	if (matrix->m != perm->m)
		return -1;

	if (matrix->kind != BFM_MATRIX_KIND_FULL)
		return -1;

	// copy old matrix to copy from

	bfm_matrix_t __attribute__((cleanup(bfm_matrix_destroy))) old = { 0 };

	if (bfm_matrix_full_create(&old, state, matrix->major, matrix->m) < 0)
		return -1;

	if (bfm_matrix_copy(&old, matrix) < 0)
		return -1;

	// actually permute matrix

	for (size_t i = 0; i < matrix->m; i++) {
		for (size_t j = 0; j < matrix->m; j++) {
			size_t const perm_i = cur_perm[i];
			size_t const perm_j = cur_perm[j];

			double const val = bfm_matrix_get(&old, i, j);
			bfm_matrix_set(matrix, perm_i, perm_j, val);
		}
	}

	return 0;
}

typedef struct {
	size_t i;
	size_t deg;
} rcm_node_t;

static int cmp_deg(void const* _a, void const* _b) {
	rcm_node_t* const a = (void*) _a;
	rcm_node_t* const b = (void*) _b;

	return (int) a->deg - (int) b->deg;
}

int bfm_perm_rcm(bfm_perm_t* perm, bfm_matrix_t* A) {
	int rv = -1;

	bfm_state_t* const state = perm->state;
	size_t const n = A->m;

	// permutation object must have the same size as the matrix

	if (perm->m != n)
		goto err_size;

	// Cuthill-McKee algorithm
	// start by getting a vector of all the degrees of the nodes in the adjacency matrix

	size_t* const degs = state->alloc(n * sizeof *degs);

	if (degs == NULL)
		goto err_degs_alloc;

	memset(degs, 0, n * sizeof *degs);

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			degs[i] += !!bfm_matrix_get(A, i, j);
		}
	}

	// create a queue of maximum size n for BFS later
	// we can create only once because it will be emptied on each BFS
	// we don't need to zero out the queue

	bool* const in_queue = state->alloc(n * sizeof *in_queue);

	if (in_queue == NULL)
		goto err_in_queue_alloc;

	memset(in_queue, 0, n * sizeof *in_queue);

	size_t* const queue = state->alloc(n * sizeof *queue);

	if (queue == NULL)
		goto err_queue_alloc;

	size_t queue_start = 0;
	size_t queue_end = 0;

#define QUEUE_INDEX(i) (queue[(i) % n])

#define PUSH_QUEUE(val) { \
	in_queue[val] = true; \
	QUEUE_INDEX(queue_end) = val; \
	queue_end++; \
}

#define POP_QUEUE(res) { \
	res = QUEUE_INDEX(queue_start); \
	in_queue[res] = false; \
	queue_start++; \
}

	// for later, doesn't need to be zeroed out

	rcm_node_t* const to_sort = state->alloc(n * sizeof *to_sort);

	if (to_sort == NULL)
		goto err_to_sort_alloc;

	// create map of visited nodes

	bool* const visited = state->alloc(n * sizeof *visited);

	if (visited == NULL)
		goto err_visited_alloc;

	memset(visited, 0, n * sizeof *visited);

	// allocate permutation vector
	// we do this as late as possible to ease error handling
	// this doesn't need to be zeroed out

	perm->perm = state->alloc(n * sizeof *perm->perm);

	if (perm->perm == NULL)
		goto err_perm_alloc;

	// continue while there are still unvisited nodes

	size_t unvisited_count = n;

	for (size_t p = 0; unvisited_count && p < n;) {
		// find the unvisited node with the smallest degree
		// min_deg_i is guaranteed to be initialized because at least one of the nodes is unvisited

		size_t min_deg = -1;
		size_t min_deg_i = 0; // XXX we don't care if this is initialized or not, but GCC complains if we don't

		for (size_t i = 0; i < n; i++) {
			if (visited[i])
				continue;

			// important that this is <= so we get the *last* one!

			if (degs[i] <= min_deg) {
				min_deg = degs[i];
				min_deg_i = i;
			}
		}

		// BFS starting from unvisited node of smallest degree

		PUSH_QUEUE(min_deg_i);

		while (queue_start != queue_end) {
			// starting from this node

			size_t cur;
			POP_QUEUE(cur);

			if (!visited[cur]) {
				visited[cur] = true;
				unvisited_count--;
			}

			// create list of nodes to sort by degree

			size_t to_sort_count = 0;

			// visit neighbouring nodes, starting with the smallest degrees
			// use heapsort(3) to guarantee O(n log n) complexity

			for (size_t i = 0; i < n; i++) {
				if (visited[i])
					continue;

				if (!bfm_matrix_get(A, cur, i))
					continue;

				rcm_node_t* const node = &to_sort[to_sort_count++];

				node->i = i;
				node->deg = degs[i];

				visited[i] = true;
			}

			qsort(to_sort, to_sort_count, sizeof *to_sort, cmp_deg);

			// add sorted nodes to queue in order

			for (size_t i = 0; i < to_sort_count; i++)
				PUSH_QUEUE(to_sort[i].i);

			// add to permutation vector, reversed

			perm->perm[n - p++ - 1] = cur;
		}
	}

	// create inverted permutation vector

	perm->inv_perm = state->alloc(n * sizeof *perm->inv_perm);

	if (perm->inv_perm == NULL) {
		state->free(perm->perm);
		goto err_inv_perm_alloc;
	}

	for (size_t i = 0; i < n; i++)
		perm->inv_perm[perm->perm[i]] = i;

	// success

	perm->has_perm = true;
	rv = 0;

err_inv_perm_alloc:
err_perm_alloc:

	state->free(visited);

err_visited_alloc:

	state->free(to_sort);

err_to_sort_alloc:

	state->free(queue);

err_queue_alloc:

	state->free(in_queue);

err_in_queue_alloc:

	state->free(degs);

err_degs_alloc:
err_size:

	return rv;
}
