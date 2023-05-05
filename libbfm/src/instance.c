#include <string.h>

#include <bfm/instance.h>

int bfm_instance_create(bfm_instance_t* instance, bfm_state_t* state, bfm_obj_t* obj) {
	memset(instance, 0, sizeof *instance);

	instance->state = state;
	instance->obj = obj;

	bfm_mesh_t* const mesh = obj->mesh;
	instance->n_effects = mesh->n_nodes * mesh->dim;

	size_t const size = instance->n_effects * sizeof *instance->effects;
	instance->effects = state->alloc(size);

	if (instance->effects == NULL)
		return -1;

	memset(instance->effects, 0, size);

	return 0;
}

int bfm_instance_destroy(bfm_instance_t* instance) {
	bfm_state_t* const state = instance->state;

	if (instance->effects)
		state->free(instance->effects);

	if (instance->conditions)
		state->free(instance->conditions);

	return 0;
}

int bfm_instance_set_n_conditions(bfm_instance_t* instance, size_t n_conditions) {
	bfm_state_t* const state = instance->state;
	instance->n_conditions = n_conditions;

	if (instance->conditions)
		state->free(instance->conditions);

	size_t const size = n_conditions * sizeof *instance->conditions;
	instance->conditions = state->alloc(size);

	if (instance->conditions == NULL)
		return -1;

	memset(instance->conditions, 0, size);

	return 0;
}

int bfm_instance_add_condition(bfm_instance_t* instance, bfm_condition_t* condition) {
	bfm_state_t* const state = instance->state;

	instance->conditions = state->realloc(instance->conditions, ++instance->n_conditions * sizeof *instance->conditions);

	if (!instance->conditions)
		return -1;

	instance->conditions[instance->n_conditions - 1] = condition;

	return 0;
}
