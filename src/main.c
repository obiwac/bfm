#include <bfm/bfm.h>
#include <bfm/mesh.h>
#include <bfm/matrix.h>
#include <bfm/obj.h>
#include <bfm/condition.h>
#include <bfm/material.h>
#include <bfm/instance.h>
#include <bfm/sim.h>

int main(void) {
    bfm_state_t state;
    bfm_state_create(&state);

    bfm_mesh_t mesh;
    int res = bfm_mesh_read_lepl1110(&mesh, &state, "../meshes/8.lepl1110");
    if (res != 0)
        return EXIT_FAILURE;

    bfm_material_t material;
    bfm_material_create(&material, &state, "base", 7.8500000e+03, 2.1100000e+11, 3.0000000e-01);

    bfm_obj_t obj;
    bfm_obj_create(&obj, &state, &mesh, &material);

    bfm_instance_t instance;
    bfm_instance_create(&instance, &state, &obj);

    bfm_sim_t sim;
    bfm_sim_create(&sim, &state, BFM_SIM_KIND_DEFORMATION);
    bfm_sim_add_instance(&sim, &instance);

    bfm_force_t force;
    bfm_force_create(&force, &state, 3);
    bfm_vec_t g; bfm_vec_create(&g, &state, 3); g.data[1] = -9.81;
    bfm_force_set_linear(&force, &g);
    bfm_vec_destroy(&g);

    bfm_sim_add_force(&sim, &force);

    bfm_sim_run(&sim);

    bfm_material_destroy(&material);
    bfm_instance_destroy(&instance);
    bfm_force_destroy(&force);
    bfm_sim_destroy(&sim);
    bfm_mesh_destroy(&mesh);
    bfm_state_destroy(&state);

	return EXIT_SUCCESS;
}
