#include "parse.h"


int bfm_parse_problem_file(char* filename, bfm_problem_t* problem) {
    FILE* file = fopen(filename, "r");
    if (file == NULL)
        return -1;

    char type[256];
    // TODO deal with read error !!!!
    fscanf(file, "Type of problem : %s\n", type);
    if (strncmp(type, "planar strains", 14) == 0)
        problem->type = BFM_PLANAR_STRAINS;
    fscanf(file, "Young modulus : %lf\n", problem->young_modulus);
    fscanf(file, "Poisson ratio : %lf\n", problem->poisson_ratio);
    fscanf(file, "Mass density : %lf\n", problem->mass_density);
    fscanf(file, "Gravity : %lf\n", problem->gravity);
    fclose(file);
}


int bfm_parse_problem_file(char* filename, bfm_problem_t* problem) {
    bfm_mesh_t *mesh = calloc(1, sizeof(problem->mesh));
    problem->mesh = mesh;
    size_t trash;

    FILE* file = fopen(filename, "r");
    fscanf(file, "Number of nodes %d\n", mesh->n_nodes);
    
    for (size_t i = 0; i < mesh->n_nodes; i++) {
        fscanf(file, "%d : %lf %lf",&trash, mesh->coords[i*2], mesh->coords[i*2+1]);
    
    char str[256];
    fscanf(file, "Number of %s : %d\n", str, mesh->n_elems);
    if (strncmp(str, "quads", 5) == 0) {
        mesh->type = BFM_QUADS;
        mesh->elems = malloc(4 * mesh->n_elems * sizeof(double));
        for (size_t i = 0; i < mesh->n_elems; i++) {
            fscanf(file, "%d : %d %d %d %d\n", &trash, &(mesh->elems[i * 4]), &(mesh->elems[i * 4 + 1]), &(mesh->elems[i * 4 + 2]), &(mesh->elems[i * 4 + 3]));
        }

    }
    else if (strncmp(str, "triangles", 9) == 0) {
        mesh->type = BFM_TRIANGLES;
        mesh->elems = malloc(3 * mesh->n_elems * sizeof(double));
        for (size_t i = 0; i < mesh->n_elems; i++) {
            fscanf(file, "%d : %d %d %d\n", &trash, &(mesh->elems[i * 3]), &(mesh->elems[i * 3 + 1]), &(mesh->elems[i * 3 + 2]));
        }
    }
    }

    fclose(file);
    return 0;
}

int bfm_free_mesh_file(bfm_problem_t* problem) {
    bfm_mesh_t* mesh = problem->mesh;
    free(mesh->elems);
    free(mesh->coords);
    free(mesh);
    return 0;
}