#pragma once

#include <bfm/condition.h>
#include <bfm/instance.h>
#include <bfm/material.h>
#include <bfm/mesh.h>
#include <bfm/obj.h>
#include <bfm/rule.h>
#include <bfm/sim.h>

typedef struct {
	bfm_state_t* state;
	bfm_mesh_t* mesh;

	size_t n_conditions;
	bfm_condition_t* conditions;

	bfm_force_t gravity;
	bfm_material_t material;
	bfm_rule_t rule;
	bfm_obj_t obj;
	bfm_instance_t instance;
	bfm_sim_t sim;
} bfm_ez_lepl1110_t;

int bfm_ez_lepl1110_create(bfm_ez_lepl1110_t* ez, bfm_state_t* state, bfm_mesh_t* mesh, char* name);
int bfm_ez_lepl1110_destroy(bfm_ez_lepl1110_t* ez);

int bfm_ez_lepl1110_write(bfm_ez_lepl1110_t* ez, size_t shift, char const* filename);
