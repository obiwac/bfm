#pragma once

#include <bfm/bfm.h>

typedef struct {
	double r;
	double g;
	double b;
	double a;
} bfm_colour_t;

typedef struct {
	bfm_state_t* state;

	char* name;
	bfm_colour_t colour;

	double rho; // density
	double E;   // Young's modulus (at room temperature)
	double nu;  // Poisson's ratio
} bfm_material_t;

int bfm_material_create(bfm_material_t* material, bfm_state_t* state, char* name, double rho, double E, double nu);
int bfm_material_destroy(bfm_material_t* material);

int bfm_material_set_colour(bfm_material_t* material, double r, double g, double b, double a);
