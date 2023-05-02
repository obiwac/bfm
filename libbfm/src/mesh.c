#include <bfm/mesh.h>

static int get_local_element(bfm_mesh_t* mesh, size_t e, bfm_local_element_t* element) {
	size_t const n_local_nodes = mesh->n_local_nodes;

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

int bfm_build_elasticity_system(bfm_mesh_t* mesh, bfm_system_t* system) {
	bfm_matrix_t* A = system->A;

	bfm_local_element_t element = {
		.n_local_nodes = mesh->n_local_nodes,
	};

	for (size_t i = 0; i < mesh->n_elems; i++) {
		get_local_element(mesh, i, &element);
	}

	return 0;
}


int bfm_build_elasticity_system_local(bfm_local_element_t* element, bfm_rule_t* rule, bfm_matrix_t* A, bfm_vec_t* B) {
	(void) A;
	(void) B;

	double* const x = element->x;
	double* const y = element->y;

	double* const dphi_dxsi = rule->dphi_dxsi;
	double* const dphi_deta = rule->dphi_deta;

	size_t* const map = element->map;

	for (size_t i = 0; i < element->n_local_nodes; i++) {
		double const weight = rule->weights[i];

		double const xsi = rule->xsi[i];
		double const eta = rule->eta[i];

		(void) weight;

		(void) xsi;
		(void) eta;

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

		double det_jacobian = fabs(dx_dxsi*dy_deta - dx_deta*dy_dxsi);

		double dphi_dx[4];
		double dphi_dy[4];

		for (size_t j = 0; j < element->n_local_nodes; j++) {
			dphi_dx[j] = (dphi_dxsi[j] * dy_deta - dphi_deta[j] * dx_dxsi) / det_jacobian;
			dphi_dy[j] = (dphi_deta[j] * dx_dxsi - dphi_dxsi[j] * dx_deta) / det_jacobian;
		}

		for (size_t j = 0; j < element->n_local_nodes; j++) {
			// int const index_i = 2 * map[j] + 1;
			// B[index_i] += det_jacobian * weight * phi[j] * -g * rho;
		}

		for (size_t j = 0; j < element->n_local_nodes; j++) {
			size_t const index_i = 2 * map[j];

			(void) index_i;

			for (size_t k = 0; k < element->n_local_nodes; k++) {
				size_t const index_j = 2 * map[k];

				(void) index_j;

				// double const f_11 = a * dphi_dx[j] * dphi_dx[k] + c * dphi_dy[j] * dphi_dy[k];
				// double const f_12 = b * dphi_dx[j] * dphi_dy[k] + c * dphi_dy[j] * dphi_dx[k];
				// double const f_21 = b * dphi_dy[j] * dphi_dx[k] + c * dphi_dx[j] * dphi_dy[k];
				// double const f_22 = a * dphi_dy[j] * dphi_dy[k] + c * dphi_dx[j] * dphi_dx[k];

				// A[index_i + 0][index_j + 0] += det_jacobian * weight * f_11;
				// A[index_i + 0][index_j + 1] += det_jacobian * weight * f_12;
				// A[index_i + 1][index_j + 0] += det_jacobian * weight * f_21;
				// A[index_i + 1][index_j + 1] += det_jacobian * weight * f_22;   
			}
		}
	}

	return 0;
}
