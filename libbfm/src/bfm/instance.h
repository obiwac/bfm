#pragma once

#include <bfm/condition.h>
#include <bfm/obj.h>

typedef struct {
	bfm_state_t* state;
	bfm_obj_t* obj;

	// one effect per coordinate of obj->mesh

	size_t n_effects;
	double* effects;

	size_t n_conditions;
	bfm_condition_t** conditions;
} bfm_instance_t;

int bfm_instance_create(bfm_instance_t* instance, bfm_state_t* state, bfm_obj_t* obj);
int bfm_instance_destroy(bfm_instance_t* instance);

int bfm_instance_set_n_conditions(bfm_instance_t* instance, size_t n_conditions);
int bfm_instance_add_condition(bfm_instance_t* instance, bfm_condition_t* condition);
