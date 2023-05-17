#include <bfm/shape.h>

static int default_phi(bfm_shape_t* shape, double* point, double* phi) {
	if (shape->dim != 2)
		return -1;

	double const xsi = point[0];
	double const eta = point[1];

	if (shape->kind == BFM_ELEM_KIND_SIMPLEX) {
		phi[0] = 1 - xsi - eta;
		phi[1] = xsi;
		phi[2] = eta;

		return 0;
	}

	if (shape->kind == BFM_ELEM_KIND_QUAD) {
		phi[0] = (1 + xsi) * (1 + eta) / 4;
		phi[1] = (1 - xsi) * (1 + eta) / 4;
		phi[2] = (1 - xsi) * (1 - eta) / 4;
		phi[3] = (1 + xsi) * (1 - eta) / 4;

		return 0;
	}

	if (shape->kind == BFM_ELEM_KIND_QUADRATIC_TRIANGLES) {
		phi[0] = 1 - 3 * (xsi + eta) + 2 * (xsi + eta) * (xsi + eta);
		phi[1] = xsi * (2 * xsi - 1);
		phi[2] = eta * (2 * eta - 1);
		phi[3] = 4 * xsi * (1 - xsi - eta);
		phi[4] = 4 * xsi * eta;
		phi[5] = 4 * eta * (1 - xsi - eta);
	}

	return -1;
}

static int default_dphi(bfm_shape_t* shape, size_t wrt, double* point, double* dphi) {
	if (shape->dim != 2)
		return -1;

	double const xsi = point[0];
	double const eta = point[1];

	if (shape->kind == BFM_ELEM_KIND_SIMPLEX) {
		dphi[0] = -1;
		dphi[1] = wrt == 0 ? 1 : 0;
		dphi[2] = wrt == 0 ? 0 : 1;

		return 0;
	}

	if (shape->kind == BFM_ELEM_KIND_QUAD) {
		dphi[0] = (wrt == 0 ?  1 + eta :  1 + xsi) / 4;
		dphi[1] = (wrt == 0 ? -1 - eta :  1 - xsi) / 4;
		dphi[2] = (wrt == 0 ? -1 + eta : -1 + xsi) / 4;
		dphi[3] = (wrt == 0 ?  1 - eta : -1 - xsi) / 4;

		return 0;
	}

	if (shape->kind == BFM_ELEM_KIND_QUADRATIC_TRIANGLES) {
		if (wrt == 0) {
			dphi[0] = -3 + 4 * (xsi + eta);
			dphi[1] = 4 * xsi - 1;
			dphi[2] = 0;
			dphi[3] = 4 - 8 * xsi - 4 * eta;
			dphi[4] = 4 * eta;
			dphi[5] = - 4 * eta;
		}
		else {
			dphi[0] =  -3 + 4 * (xsi + eta);
			dphi[1] = 0;
			dphi[2] = 4 * eta - 1;
			dphi[3] = - 4 * xsi;
			dphi[4] = 4 * xsi;
			dphi[5] = 4 - 4 * xsi - 8 * eta;
		}
	}

	return -1;
}

int bfm_shape_create(bfm_shape_t* shape, bfm_state_t* state, size_t dim, bfm_elem_kind_t kind) {
	shape->state = state;

	shape->dim = dim;
	shape->kind = kind;

	// TODO support different shape functions
	//      currently, only 2D 3/4 point serendipity shape functions are supported

	shape->phi = default_phi;
	shape->dphi = default_dphi;

	return 0;
}

int bfm_shape_destroy(bfm_shape_t* shape) {
	(void) shape;
	return 0;
}
