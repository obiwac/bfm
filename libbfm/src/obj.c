#include <bfm/obj.h>

int bfm_obj_create(bfm_obj_t* obj, bfm_state_t* state, bfm_mesh_t* mesh, bfm_material_t* material, bfm_rule_t* rule) {
	obj->state = state;

	obj->mesh = mesh;
	obj->material = material;
	obj->rule = rule;

	return 0;
}

int bfm_obj_destroy(bfm_obj_t* obj) {
	(void) obj;
	return 0;
}
