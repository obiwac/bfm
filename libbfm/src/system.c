#include <bfm/system.h>

int bfm_system_create(bfm_system_t* system, bfm_state_t* state, size_t n) {
	system->state = state;
	system->kind = BFM_SYSTEM_KIND_GENERIC;

	system->n = n;

	if (bfm_matrix_full_create(&system->A, state, BFM_MATRIX_MAJOR_ROW, n) < 0)
		return -1;

	if (bfm_vec_create(&system->b, state, n) < 0) {
		bfm_matrix_destroy(&system->A);
		return -1;
	}

	return 0;
}

int bfm_system_destroy(bfm_system_t* system) {
	bfm_matrix_destroy(&system->A);
	bfm_vec_destroy(&system->b);

	return 0;
}

// elasticity systems
// only 2D simplex or quad meshes are supported

typedef struct {
	bfm_elem_kind_t kind;

	size_t map[4];
	size_t coord[2][4];
} elem_t;

static void get_elem(elem_t* elem, bfm_mesh_t* mesh, size_t i) {
	bfm_elem_kind_t const kind = mesh->kind;
	size_t const dim = mesh->dim;

	elem->kind = kind;

	for (size_t j = 0; j < kind; j++) {
		size_t const map = mesh->elems[kind * i + j];
		elem->map[j] = map;

		for (size_t k = 0; k < dim; k++)
			elem->coord[k][j] = mesh->coords[map * dim + k];
	}
}

static void fill_elasticity_elem(elem_t* elem, bfm_system_t* system, bfm_instance_t* instance, size_t n_forces, bfm_force_t** forces) {
	bfm_obj_t* const obj = instance->obj;
   bfm_material_t* const material = obj->material;
	bfm_rule_t* const rule = obj->rule;

	// constants

   double const a = material->E / (1 - material->nu * material->nu);
	double const b = material->E * material->nu / (1 - material->nu * material->nu);
	double const c = material->E / (2 * (1 + material->nu));

	// go through integration points

	for (size_t i = 0; i < rule->n_points; i++) {
		// TODO integration point i
	}
}

static void apply_constraint(bfm_system_t* system, size_t node, double value) {
	// TODO deal with band matrices

	for (size_t i = 0; i < system->A.m; i++) {
		// TODO deal with non-zero conditions

		system->b.data[i] -= value * bfm_matrix_get(&system->A, i, node);
		bfm_matrix_set(&system->A, i, node, 0);
	}

	for (size_t i = 0; i < system->A.m; i++)
		bfm_matrix_set(&system->A, node, i, 0);

	bfm_matrix_set(&system->A, node, node, 1);
	system->b.data[node] = value;
}

int bfm_system_create_elasticity(bfm_system_t* system, bfm_instance_t* instance, size_t n_forces, bfm_force_t** forces) {
	bfm_state_t* const state = instance->state;
	bfm_obj_t* const obj = instance->obj;
	bfm_mesh_t* const mesh = obj->mesh;
   bfm_material_t* const material = obj->material;
	size_t const n = mesh->n_nodes * mesh->dim;

	// check that mesh is supported

	if (mesh->dim != 2)
		return -1;

	if (mesh->kind != BFM_ELEM_KIND_SIMPLEX && mesh->kind != BFM_ELEM_KIND_QUAD)
		return -1;

	// create system object

	if (bfm_system_create(system, state, n) < 0)
		return -1;

	system->kind = BFM_SYSTEM_KIND_ELASTICITY;

	// go through all elements

	elem_t elem;

	for (size_t i = 0; i < mesh->n_elems; i++) {
		get_elem(&elem, mesh, i);
		fill_elasticity_elem(&elem, system, instance, n_forces, forces);
	}

	// apply conditions

	for (size_t i = 0; i < instance->n_conditions; i++) {
		bfm_condition_t* const condition = instance->conditions[i];

		for (size_t j = 0; j < mesh->n_nodes; j++) {
			for (size_t k = 0; condition->nodes[j] && k < mesh->dim; k++)
				apply_constraint(system, j * mesh->dim + k, 0);
		}
	}

	return 0;
}
