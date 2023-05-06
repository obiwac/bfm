#include <string.h>

#include <bfm/sim.h>

int bfm_sim_create(bfm_sim_t* sim, bfm_state_t* state, bfm_sim_kind_t kind) {
	memset(sim, 0, sizeof *sim);

	sim->state = state;
	sim->kind = kind;

	return 0;
}

int bfm_sim_destroy(bfm_sim_t* sim) {
	bfm_state_t* const state = sim->state;

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

	sim->instances[sim->n_instances - 1] = instance;

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

	sim->forces[sim->n_forces - 1] = force;

	return 0;
}

// simulation run functions per kind

static int run_deformation(bfm_sim_t* sim) {
	for (size_t i = 0; i < sim->n_instances; i++) {
		bfm_instance_t* const instance = sim->instances[i];
		memset(instance->effects, 0, instance->n_effects * sizeof *instance->effects);

		bfm_obj_t* const obj = instance->obj;
		bfm_mesh_t* const mesh = obj->mesh;

		bfm_system_t system;
		
		bfm_state_t state;
		bfm_state_create(&state);

		// Allocate system, maybe write a function for this ?
		bfm_matrix_full_create(system.A, &state, BFM_MATRIX_MAJOR_ROW, mesh->n_elems * 2);
		system.B = state.alloc(sizeof *system.B);
		bfm_vec_create(system.B, &state, mesh->n_elems * 2);
		// Build the system and solve it
		bfm_build_elasticity_system(instance, sim->forces, sim->n_forces, &system);
		bfm_matrix_solve(system.A, system.B);

		for (size_t k = 0; k < mesh->n_nodes; k++) {
			instance->effects[k * 2 + 0] = system.B->data[k * 2 + 0];
			instance->effects[k * 2 + 1] = system.B->data[k * 2 + 1];
		}
		// WOuld be easier with a func for system
		bfm_vec_destroy(system.B);
		state.free(system.B);
		bfm_matrix_destroy(system.A);
		bfm_state_destroy(&state);
	}

	return 0;
}

int bfm_sim_run(bfm_sim_t* sim) {
	if (sim->kind == BFM_SIM_KIND_DEFORMATION)
		return run_deformation(sim);

	return -1;
}
