#include <stdio.h>
#include <string.h>

#include <bfm/mesh.h>
#include <bfm/sim.h>
#include <bfm/system.h>

int bfm_sim_create(bfm_sim_t* sim, bfm_state_t* state, bfm_sim_kind_t kind) {
	memset(sim, 0, sizeof *sim);

	sim->state = state;
	sim->kind = kind;

	return 0;
}

int bfm_sim_destroy(bfm_sim_t* sim) {
	bfm_state_t* const state = sim->state;

	if (sim->instances) {
		state->free(sim->instances);
	}

	if (sim->forces) {
		state->free(sim->forces);
	}

	return 0;
}

int bfm_sim_set_n_instances(bfm_sim_t* sim, size_t n_instances) {
	bfm_state_t* const state = sim->state;
	sim->n_instances = n_instances;

	if (sim->instances) {
		state->free(sim->instances);
	}

	size_t const size = n_instances * sizeof *sim->instances;
	sim->instances = state->alloc(size);

	if (sim->instances == NULL) {
		return -1;
	}

	memset(sim->instances, 0, size);

	return 0;
}

int bfm_sim_add_instance(bfm_sim_t* sim, bfm_instance_t* instance) {
	bfm_state_t* const state = sim->state;

	sim->instances = state->realloc(sim->instances, ++sim->n_instances * sizeof *sim->instances);

	if (!sim->instances) {
		return -1;
	}

	sim->instances[sim->n_instances - 1] = instance;

	return 0;
}

int bfm_sim_set_n_forces(bfm_sim_t* sim, size_t n_forces) {
	bfm_state_t* const state = sim->state;
	sim->n_forces = n_forces;

	if (sim->forces) {
		state->free(sim->forces);
	}

	size_t const size = n_forces * sizeof *sim->forces;
	sim->forces = state->alloc(size);

	if (sim->forces == NULL) {
		return -1;
	}

	memset(sim->forces, 0, size);

	return 0;
}

int bfm_sim_add_force(bfm_sim_t* sim, bfm_force_t* force) {
	bfm_state_t* const state = sim->state;

	sim->forces = state->realloc(sim->forces, ++sim->n_forces * sizeof *sim->forces);

	if (!sim->forces) {
		return -1;
	}

	sim->forces[sim->n_forces - 1] = force;

	return 0;
}

// simulation run functions per kind

typedef int (*system_create_elasticity_fn_t)(bfm_system_t* system, bfm_instance_t* instance, size_t n_forces, bfm_force_t** forces);

static int run_elasticity(bfm_sim_t* sim, system_create_elasticity_fn_t system_create_fn) {
	for (size_t i = 0; i < sim->n_instances; i++) {
		bfm_instance_t* const instance = sim->instances[i];
		bfm_obj_t* const obj = instance->obj;
		bfm_mesh_t* const mesh = obj->mesh;
		size_t const dim = mesh->dim;

		// create and solve elasticity system

		bfm_system_t __attribute__((cleanup(bfm_system_destroy))) system;

		if (system_create_fn(&system, instance, sim->n_forces, sim->forces) < 0) {
			return -1;
		}

		if (bfm_system_renumber(&system) < 0) {
			return -1;
		}

		bfm_matrix_solve(&system.A, &system.b);
		bfm_perm_perm_vec(&system.perm, &system.b, true);

		// set instance effects to result of equation

		for (size_t j = 0; j < mesh->n_nodes; j++) {
			for (size_t k = 0; k < dim; k++) {
				instance->effects[j * dim + k] = system.b.data[j * dim + k];
			}
		}
	}

	return 0;
}

int bfm_sim_run(bfm_sim_t* sim) {
	if (sim->kind == BFM_SIM_KIND_NONE) {
		return 0;
	}

	if (sim->kind == BFM_SIM_KIND_PLANAR_STRAIN) {
		return run_elasticity(sim, bfm_system_create_planar_strain);
	}

	if (sim->kind == BFM_SIM_KIND_PLANAR_STRESS) {
		return run_elasticity(sim, bfm_system_create_planar_stress);
	}

	if (sim->kind == BFM_SIM_KIND_AXISYMMETRIC_STRAIN) {
		return run_elasticity(sim, bfm_system_create_axisymmetric_strain);
	}

	return -1;
}
