#pragma once

#include <stdlib.h>
#include <mesh.h>

int bfm_parse_problem_file(char* filename, bfm_problem_t* problem);

int bfm_parse_mesh_file(char* filename, bfm_mesh_t* mesh);

int bfm_parse_output_file(char *filename, bfm_mesh_t* problem);
