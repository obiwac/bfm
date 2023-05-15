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
