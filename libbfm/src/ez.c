#include <stdio.h>
#include <string.h>

#include <bfm/ez.h>

int bfm_ez_lepl1110_create(bfm_ez_lepl1110_t* ez, bfm_state_t* state, bfm_mesh_t* mesh, char* name) {
	// TODO error messages & more error checking (alloc's/fscanf's)
	// TODO I think a few things aren't freed correctly on error

	ez->state = state;
	ez->mesh = mesh;

	// create needed structures, some with dummy values
	// these will be replaced as we read the problem file

	if (bfm_sim_create(&ez->sim, state, 0) < 0)
		return -1;

	if (bfm_material_create(&ez->material, state, "lepl1110", 0, 0, 0) < 0)
		return -1;

	if (bfm_rule_create_gauss_legendre(&ez->rule, state, 2, mesh->kind) < 0)
		return -1;

	if (bfm_obj_create(&ez->obj, state, mesh, &ez->material, &ez->rule) < 0)
		return -1;

	if (bfm_instance_create(&ez->instance, state, &ez->obj) < 0)
		return -1;

	bfm_sim_add_instance(&ez->sim, &ez->instance);

	FILE* const fp = fopen(name, "r");

	if (fp == NULL)
		return -1;

	char line[50];
	char arg[50];
	char arg2[50];

	// from LEPL1110 fem.c

	while (!feof(fp)) {
		fscanf(fp, "%19[^\n]s \n", line);

		if (strncasecmp(line, "Type of problem     ", 19) == 0) {
			fscanf(fp, ":  %[^\n]s \n", arg);

			if (strncasecmp(arg, "Planar strains", 13) == 0)
			   ez->sim.kind = BFM_SIM_KIND_DEFORMATION;

			else if (strncasecmp(arg, "Planar stresses", 13) == 0)
			   ez->sim.kind = BFM_SIM_KIND_PLANAR_STRAINS;

			else if (strncasecmp(arg, "Axi-symetric problem", 13) == 0)
			   ez->sim.kind = BFM_SIM_KIND_AXISYMETRIC;
		}

		else if (strncasecmp(line, "Young modulus       ", 19) == 0)
			fscanf(fp, ":  %le\n", &ez->material.E);

		else if (strncasecmp(line, "Poisson ratio       ", 19) == 0)
			fscanf(fp, ":  %le\n", &ez->material.nu);

		else if (strncasecmp(line, "Mass density        ", 19) == 0)
			fscanf(fp, ":  %le\n", &ez->material.rho);

		else if (strncasecmp(line, "Gravity             ", 19) == 0) {
			if (bfm_force_create(&ez->gravity, state, 2) < 0)
				return -1;

			bfm_vec_t* const vec = state->alloc(sizeof *vec);

			if (!vec)
				return -1;

			if (bfm_vec_create(vec, state, 2) < 0)
				return -1;

			fscanf(fp, ":  %le\n", &vec->data[1]);
			vec->data[1] *= -1;

			bfm_force_set_linear(&ez->gravity, vec);

			bfm_vec_destroy(vec);
			state->free(vec);

			bfm_sim_add_force(&ez->sim, &ez->gravity);
		}

		else if (strncasecmp(line, "Boundary condition  ", 19) == 0) {
			double val;
			fscanf(fp, ":  %19s = %le : %[^\n]s\n", arg, &val, arg2);

			bfm_condition_kind_t kind;

			if (strncasecmp(arg, "Dirichlet-X", 19) == 0)
				kind = BFM_CONDITION_KIND_DIRICHLET_X;

			else if (strncasecmp(arg, "Dirichlet-Y", 19) == 0)
				kind = BFM_CONDITION_KIND_DIRICHLET_Y;

			else if (strncasecmp(arg, "Neumann-X", 19) == 0)
				kind = BFM_CONDITION_KIND_NEUMANN_X;

			else if (strncasecmp(arg, "Neumann-Y", 19) == 0)
				kind = BFM_CONDITION_KIND_NEUMANN_Y;

			else
				return -1;

			ez->conditions = state->realloc(ez->conditions, ++ez->n_conditions * sizeof *ez->conditions);
			bfm_condition_t* const condition = &ez->conditions[ez->n_conditions - 1];

			if (bfm_condition_create(condition, state, mesh, kind) < 0)
				return -1;

			condition->value = val;

			for (size_t i = 0; i < mesh->n_domains; i++) {
				if (strncasecmp(mesh->domains[i].name, arg2, 25))
					continue;

				bfm_domain_t* const domain = &mesh->domains[i];

				for (size_t j = 0; j < domain->n_elements; j++)
					condition->nodes[domain->elements[j]] = true;

				break;
			}
		}

		fscanf(fp, "\n");
	}

	fclose(fp);

	// add conditions to instance
	// we do this after because pointers to conditions will change when reallocing

	for (size_t i = 0; i < ez->n_conditions; i++)
		bfm_instance_add_condition(&ez->instance, &ez->conditions[i]);

	return 0;
}

int bfm_ez_lepl1110_destroy(bfm_ez_lepl1110_t* ez) {
	bfm_state_t* const state = ez->state;

	for (size_t i = 0; i < ez->n_conditions; i++) {
		bfm_condition_t* const condition = &ez->conditions[i];
		bfm_condition_destroy(condition);
	}

	state->free(ez->conditions);

	bfm_force_destroy(&ez->gravity);
	bfm_material_destroy(&ez->material);
	bfm_rule_destroy(&ez->rule);
	bfm_obj_destroy(&ez->obj);
	bfm_instance_destroy(&ez->instance);
	bfm_sim_destroy(&ez->sim);

	return 0;
}

int bfm_ez_lepl1110_write(bfm_ez_lepl1110_t* ez, size_t shift, char const* filename) {
	FILE* const fp = fopen(filename, "w");
	
	if (!fp)
		return -1;

	fprintf(fp, "Number of nodes %zu\n", ez->obj.mesh->n_nodes);
	for (size_t i = 0; i < ez->obj.mesh->n_nodes; i++) {
		fprintf(fp, "%14.7e", ez->instance.effects[i * 2 + shift]);
		if (i + 1 != ez->instance.n_effects && (i + 1) % 3 == 0)
			fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
	fclose(fp);
	
	return 0;
}