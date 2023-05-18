#pragma once

#include <bfm/force.h>
#include <bfm/instance.h>

typedef enum {
	BFM_SIM_KIND_NONE                = 0,
	BFM_SIM_KIND_PLANAR_STRAIN       = 1, // deplacement
	BFM_SIM_KIND_PLANAR_STRESS       = 2, // force
	BFM_SIM_KIND_AXISYMMETRIC_STRAIN = 3, // deplacement
} bfm_sim_kind_t;

typedef struct {
	bfm_state_t* state;
	bfm_sim_kind_t kind;

	size_t n_instances;
	bfm_instance_t** instances;

	size_t n_forces;
	bfm_force_t** forces;
} bfm_sim_t;

int bfm_sim_create(bfm_sim_t* sim, bfm_state_t* state, bfm_sim_kind_t kind);
int bfm_sim_destroy(bfm_sim_t* sim);

int bfm_sim_set_n_instances(bfm_sim_t* sim, size_t n_instances);
int bfm_sim_add_instance(bfm_sim_t* sim, bfm_instance_t* instance);

int bfm_sim_set_n_forces(bfm_sim_t* sim, size_t n_forces);
int bfm_sim_add_force(bfm_sim_t* sim, bfm_force_t* force);

int bfm_sim_run(bfm_sim_t* sim);
