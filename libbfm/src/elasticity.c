#include <bfm/elasticity.h>

static int get_local_element(bfm_mesh_t* mesh, size_t e, bfm_local_element_t* element) {
	size_t const n_local_nodes = mesh->kind;

	size_t* const elems = mesh->elems;

	size_t* const map = element->map;
	double* const x = element->x;
	double* const y = element->y;

	for (size_t i = 0; i < n_local_nodes; i++) {
		map[i] = elems[e * n_local_nodes + i];

		x[i] = mesh->coords[map[i] * 2];
		y[i] = mesh->coords[map[i] * 2 + 1];
	}
	 return 0;
}

static int build_elasticity_system_local(bfm_local_element_t* element, bfm_instance_t* instance, bfm_force_t** forces, size_t n_forces, bfm_system_t* system, double a, double b, double c) {
	bfm_obj_t* const obj = instance->obj;
   bfm_mesh_t* const mesh = obj->mesh;

	double* const x = element->x;
	double* const y = element->y;

	bfm_rule_t* const rule = &mesh->rule;
	
	bfm_shape_functions_t* const shape = &mesh->functions;
	
	bfm_matrix_t* const A = system->A;
	bfm_vec_t* const B = system->B;

	size_t* const map = element->map;

	bfm_vec_t pos;
	bfm_vec_t force_on_point;
	bfm_vec_create(&pos, instance->state, 3);
	bfm_vec_create(&force_on_point, instance->state, 3);

	for (size_t i = 0; i < element->n_local_nodes; i++) {
		pos.data[0] = x[i];
		pos.data[1] = y[i];

		double const weight = rule->weights[i];

		double const xsi = rule->xsi[i];
		double const eta = rule->eta[i];
		
		double phi[4];
		shape->get_phi(xsi, eta, phi);
		
		double dphi_dxsi[4];
		double dphi_deta[4];
		shape->get_dphidxsi(xsi, eta, dphi_dxsi);
		shape->get_dphideta(xsi, eta, dphi_deta);

		double dx_dxsi = 0;
		double dx_deta = 0;
		double dy_dxsi = 0;
		double dy_deta = 0;

		
		for (size_t k = 0; k < element->n_local_nodes; k++) {
			dx_dxsi += x[k] * dphi_dxsi[k];
			dx_deta += x[k] * dphi_deta[k];
			dy_dxsi += y[k] * dphi_dxsi[k];
			dy_deta += y[k] * dphi_deta[k];
		}

		double const det_jacobian = fabs(dx_dxsi * dy_deta - dx_deta * dy_dxsi);

		double dphi_dx[4];
		double dphi_dy[4];

		for (size_t j = 0; j < element->n_local_nodes; j++) {
			dphi_dx[j] = (dphi_dxsi[j] * dy_deta - dphi_deta[j] * dy_dxsi) / det_jacobian;
			dphi_dy[j] = (dphi_deta[j] * dx_dxsi - dphi_dxsi[j] * dx_deta) / det_jacobian;
		}
		
		for (size_t j = 0; j < element->n_local_nodes; j++) {
			size_t const index_i = 2 * map[j];

			for (size_t k = 0; k < n_forces; k++) {
				bfm_force_t* const force = forces[k];
				bfm_force_eval(force, &pos, &force_on_point);

				B->data[index_i + 0] += det_jacobian * weight * force_on_point.data[0] * obj->material->rho * phi[j];
				B->data[index_i + 1] += det_jacobian * weight * force_on_point.data[1] * obj->material->rho * phi[j];
			}
		}

		for (size_t j = 0; j < element->n_local_nodes; j++) {
			size_t const index_i = 2 * map[j];

			for (size_t k = 0; k < element->n_local_nodes; k++) {
				size_t const index_j = 2 * map[k];

				double const f_11 = a * dphi_dx[j] * dphi_dx[k] + c * dphi_dy[j] * dphi_dy[k];
				double const f_12 = b * dphi_dx[j] * dphi_dy[k] + c * dphi_dy[j] * dphi_dx[k];
				double const f_21 = b * dphi_dy[j] * dphi_dx[k] + c * dphi_dx[j] * dphi_dy[k];
				double const f_22 = a * dphi_dy[j] * dphi_dy[k] + c * dphi_dx[j] * dphi_dx[k];

				bfm_matrix_add(A, index_i, index_j, det_jacobian * weight * f_11);
				bfm_matrix_add(A, index_i, index_j + 1, det_jacobian * weight * f_11);
				bfm_matrix_add(A, index_i + 1, index_j, det_jacobian * weight * f_21);
				bfm_matrix_add(A, index_i + 1, index_j + 1, det_jacobian * weight * f_22);
			}
		}
	}

	return 0;
}

static int apply_constrain(bfm_system_t* system, size_t node, double value) {
	// TODO deal with band matrix ??
	for (size_t i = 0; i < system->A->m; i++) {
		// TODO work with cond != 0
		system->B->data[i] -= value * bfm_matrix_get(system->A, i, node);
		bfm_matrix_set(system->A, i, node, 0.);
	}

	for (size_t i = 0; i < system->A->m; i++) {
		bfm_matrix_set(system->A, node, i, 0.);
	}

	bfm_matrix_set(system->A, node, node, 1.);
	system->B->data[node] = value;
}

int bfm_build_elasticity_system(bfm_instance_t* instance, bfm_force_t** forces, size_t n_forces, bfm_system_t* system) {
	bfm_obj_t* const obj = instance->obj;
	bfm_mesh_t* const mesh = obj->mesh;
   	bfm_material_t* const material = obj->material;

   	double const a = material->E / (1 - material->nu * material->nu);
	double const b = material->E * material->nu / (1 - material->nu * material->nu);
	double const c = material->E / (2 * (1 + material->nu));

	bfm_local_element_t element = {
		.n_local_nodes = mesh->kind,
	};

	for (size_t i = 0; i < mesh->n_elems; i++) {
		get_local_element(mesh, i, &element);
		build_elasticity_system_local(&element, instance, forces, n_forces, system, a, b, c);
	}

	for (size_t i = 0; i < instance->n_conditions; i++) {
		bfm_condition_t* const condition = instance->conditions[i];
		for (size_t j = 0; j < mesh->n_nodes; j++) {
			if (condition->nodes[j]) {
				// TODO deal with Dirichlet != 0
				// apply_constrain(system, j * 2, 0);
				// apply_constrain(system, j * 2 + 1, 0);
			}
		}
	}

	return 0;
}
