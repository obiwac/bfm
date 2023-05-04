#include <string.h>

#include <bfm/sim.h>

int bfm_sim_create(bfm_sim_t* sim, bfm_state_t* state, bfm_sim_kind_t kind) {
	memset(sim, 0, sizeof *sim);

	sim->state = state;
	sim->kind = kind;

	return 0;
}

int bfm_sim_destroy(bfm_sim_t* sim) {
	bfm_state_t* const state = instance->state;

	if (sim->instances)
		state->free(sim->instances);

	if (sim->forces)
		state->free(sim->forces);

	return 0;
}

int bfm_sim_set_n_instances(bfm_sim_t* sim, size_t n_instances) {
	bfm_state_t* const state = sim->state;
	sim->n_instances = n_instances;

	if (sim->instances)
		state->free(sim->instances);

	size_t const size = n_instances * sizeof *sim->instances;
	sim->instances = state->alloc(size);

	if (sim->instances == NULL)
		return -1;

	memset(sim->instances, 0, size);

	return 0;
}

int bfm_sim_add_instance(bfm_sim_t* sim, bfm_instance_t* instance) {
	bfm_state_t* const state = sim->state;

	sim->instances = state->realloc(sim->instances, ++sim->n_instances * sizeof *sim->instances);

	if (!sim->instances)
		return -1;

	sim->instances[sim->n_instances] = NULL;

	return 0;
}

int bfm_sim_set_n_forces(bfm_sim_t* sim, size_t n_forces) {
	bfm_state_t* const state = sim->state;
	sim->n_forces = n_forces;

	if (sim->forces)
		state->free(sim->forces);

	size_t const size = n_forces * sizeof *sim->forces;
	sim->forces = state->alloc(size);

	if (sim->forces == NULL)
		return -1;

	memset(sim->forces, 0, size);

	return 0;
}

int bfm_sim_add_force(bfm_sim_t* sim, bfm_force_t* force) {
	bfm_state_t* const state = sim->state;

	sim->forces = state->realloc(sim->forces, ++sim->n_forces * sizeof *sim->forces);

	if (!sim->forces)
		return -1;

	sim->forces[sim->n_forces] = NULL;

	return 0;
}

// simulation run functions per kind

static int run_deformation(bfm_sim_t* sim) {
	(void) sim;

	// TODO

	return 0;
}

int bfm_sim_run(bfm_sim_t* sim) {
	if (sim->kind == BFM_SIM_KIND_DEFORMATION)
		return run_deformation(sim);

	return -1;
}
