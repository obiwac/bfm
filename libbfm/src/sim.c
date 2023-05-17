#include <string.h>
#include <stdio.h>

#include <bfm/sim.h>
#include <bfm/mesh.h>
#include <bfm/system.h>

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
		bfm_obj_t* const obj = instance->obj;
		bfm_mesh_t* const mesh = obj->mesh;
		size_t const dim = mesh->dim;

		// create and solve elasticity system

		bfm_system_t __attribute__((cleanup(bfm_system_destroy))) system;

		if (bfm_system_create_elasticity(&system, instance, sim->n_forces, sim->forces) < 0)
			return -1;

		bfm_matrix_solve(&system.A, &system.b);

		// set instance effects to result of equation

		for (size_t j = 0; j < mesh->n_nodes; j++) {
			for (size_t k = 0; k < dim; k++)
				instance->effects[j * dim + k] = system.b.data[j * dim + k];
		}
	}

	return 0;
}

int bfm_sim_run(bfm_sim_t* sim) {
	if (sim->kind == BFM_SIM_KIND_DEFORMATION)
		return run_deformation(sim);

	return -1;
}

int bfm_sim_read_lepl1110(bfm_sim_t* sim, bfm_mesh_t* mesh, bfm_state_t* state, char* name) {
	int rv = -1;

	sim->state = state;

	// TODO error messages & more error checking (alloc's/fscanf's)

	FILE* const fp = fopen(name, "r");

	// if (fp == NULL)
		// goto err_fopen; // TODO error message

	char line[50];
	char arg[50];
	char arg2[50];

	double val;

	double E;
	double nu;
	double rho;
	
	bfm_condition_t** conds = NULL;
	size_t n_conds = 0;

	// From lepl1110 fem.c
	while (!feof(fp)) {
        fscanf(fp, "%19[^\n]s \n", &line);
        if (strncasecmp(line, "Type of problem     ", 19) == 0) {
            fscanf(fp, ":  %[^\n]s \n", &arg);

            if (strncasecmp(arg, "Planar strains", 13) == 0)
               sim->kind = BFM_SIM_KIND_DEFORMATION;

            else if (strncasecmp(arg,"Planar stresses",13) == 0)
               sim->kind = BFM_SIM_KIND_PLANAR_STRAINS;

            else if (strncasecmp(arg, "Axi-symetric problem",13) == 0)
               sim->kind = BFM_SIM_KIND_AXISYMETRIC;
		}

        else if (strncasecmp(line, "Young modulus       ", 19) == 0)
            fscanf(fp,":  %le\n", &E);

        else if (strncasecmp(line,"Poisson ratio       ", 19) == 0)
            fscanf(fp,":  %le\n",&nu);

        else if (strncasecmp(line, "Mass density        ", 19) == 0)
            fscanf(fp,":  %le\n",&rho);

        else if (strncasecmp(line, "Gravity             ", 19) == 0) {
			bfm_force_t* force = state->alloc(sizeof(bfm_force_t));
			bfm_force_create(force, state, 2);

			bfm_vec_t* vec = state->alloc(sizeof(bfm_vec_t));
			bfm_vec_create(vec, state, 2);
            fscanf(fp, ":  %le\n", vec->data + 1);

			bfm_force_set_linear(force, vec);
			bfm_vec_destroy(vec);

			bfm_sim_add_force(sim, force);
		}

        else if (strncasecmp(line, "Boundary condition  ", 19) == 0) {
            fscanf(fp, ":  %19s = %le : %[^\n]s\n", &arg, &val, &arg2);
			bfm_condition_kind_t kind;
            if (strncasecmp(arg, "Dirichlet-X", 19) == 0)
                kind = BFM_CONDITION_KIND_DIRICHLET;

            else if (strncasecmp(arg, "Dirichlet-Y", 19) == 0)
                kind = BFM_CONDITION_KIND_DIRICHLET;

            else if (strncasecmp(arg, "Neumann-X", 19) == 0)
                kind = BFM_CONDITION_KIND_NEUMANN;

            else if (strncasecmp(arg, "Neumann-Y", 19) == 0)
                kind = BFM_CONDITION_KIND_NEUMANN;

			bfm_condition_t* condition = state->alloc(sizeof(bfm_condition_t));
			bfm_condition_create(condition, state, mesh, kind);
			condition->values = state->alloc(2 * sizeof *condition->values);
			condition->values[0] = condition->values[1] = val;

			for (size_t i = 0; i < mesh->n_domains; i++) {
				if (strncasecmp(mesh->domains[i]->name, arg2, 25) == 0) {
					bfm_domain_t* domain = mesh->domains[i];
					for (size_t j = 0; j < domain->n_elements; j++)
						condition->nodes[domain->elements[j]] = true;
					break;
				}
			}
			conds = state->realloc(conds, (n_conds + 1) * sizeof *conds);
			conds[n_conds++] = condition;
			
		}
        fscanf(fp,"\n");

	}
	fclose(fp);

	bfm_material_t* material = state->alloc(sizeof(bfm_material_t));
	bfm_material_create(material, state, "basic", E, rho, nu);

	bfm_rule_t* rule = state->alloc(sizeof(bfm_rule_t));
	bfm_rule_create(rule, state, 2, mesh->kind, mesh->kind);

	bfm_obj_t* obj = state->alloc(sizeof(bfm_obj_t));
	bfm_obj_create(obj, state, mesh, material, rule);

	bfm_instance_t* const instance = state->alloc(sizeof(bfm_instance_t));
	bfm_instance_create(instance, state, obj);
	bfm_sim_add_instance(sim, instance);

	instance->conditions = conds;
	instance->n_conditions = n_conds;

	bfm_sim_add_instance(sim, instance);
	return 0;
}
