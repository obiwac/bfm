#include <bfm/bfm.h>
#include <bfm/mesh.h>
#include <bfm/parse.h>
#include <bfm/matrix.h>
#include <bfm/parse.h>

int main(void) {
    bfm_problem_t problem;
    bfm_parse_problem_file("problem.txt", &problem);
    bfm_parse_mesh_file("mesh.txt", &problem);

    // TODO Create integration rules; maybe do it in parse_mesh_file ?
    // alloc here or here in function ? so many questions to answer



    bfm_free_mesh_file(&problem);

	 return EXIT_SUCCESS;
}
