#include <string.h>

#include <bfm/material.h>

int bfm_material_create(bfm_material_t* material, bfm_state_t* state, char* name, double rho, double E, double nu) {
	memset(material, 0, sizeof *material);
	material->state = state;

	// TODO create some kind of helper header for functions like strdup
	//      currently, this is allocating using malloc and not state->alloc!

	material->name = strdup(name);

	if (!material->name) {
		return -1;
	}

	material->rho = rho;
	material->E = E;
	material->nu = nu;

	return 0;
}

int bfm_material_destroy(bfm_material_t* material) {
	bfm_state_t* const state = material->state;

	state->free(material->name);

	return 0;
}

int bfm_material_set_colour(bfm_material_t* material, double r, double g, double b, double a) {
	material->colour.r = r;
	material->colour.g = g;
	material->colour.b = b;
	material->colour.a = a;

	return 0;
}
