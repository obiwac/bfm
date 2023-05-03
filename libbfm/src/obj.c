#include <bfm/obj.h>

int bfm_obj_create(bfm_obj_t* obj, bfm_state_t* state, bfm_mesh_t* mesh, bfm_material_t* material) {
	obj->state = state;

	obj->mesh = mesh;
	obj->material = material;

	return 0;
}

int bfm_obj_destroy(bfm_obj_t* obj) {
	(void) obj;
	return 0;
}
