#pragma once

#include <bfm/condition.h>
#include <bfm/material.h>
#include <bfm/mesh.h>
#include <bfm/rule.h>

typedef struct {
	bfm_state_t* state;

	bfm_mesh_t* mesh;
	bfm_material_t* material;
	bfm_rule_t* rule;
} bfm_obj_t;

int bfm_obj_create(bfm_obj_t* obj, bfm_state_t* state, bfm_mesh_t* mesh, bfm_material_t* material, bfm_rule_t* rule);
int bfm_obj_destroy(bfm_obj_t* obj);
