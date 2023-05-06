#pragma once

#include <bfm/mesh.h>
#include <bfm/matrix.h>
#include <bfm/math.h>
#include <bfm/instance.h>
#include <bfm/obj.h>
#include <bfm/force.h>

int bfm_build_elasticity_system(bfm_instance_t* instance, bfm_force_t** forces, size_t n_forces, bfm_system_t* system);