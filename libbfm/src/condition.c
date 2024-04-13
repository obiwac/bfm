#include <string.h>

#include <bfm/condition.h>

int bfm_condition_create(bfm_condition_t* condition, bfm_state_t* state, bfm_mesh_t* mesh, bfm_condition_kind_t kind) {
	condition->state = state;

	condition->mesh = mesh;
	condition->kind = kind;

	size_t const size = mesh->n_nodes * sizeof *condition->nodes;
	condition->nodes = state->alloc(size);

	if (!condition->nodes) {
		return -1;
	}

	memset(condition->nodes, 0, size);

	return 0;
}

int bfm_condition_destroy(bfm_condition_t* condition) {
	bfm_state_t* const state = condition->state;

	state->free(condition->nodes);

	return 0;
}
