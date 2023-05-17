cmake_minimum_required(VERSION 3.9)
project(bfm VERSION 0.1.0 DESCRIPTION "Big F'ing Matrix")

# library setup

add_library(bfm SHARED
	src/condition.c
	src/force.c
	src/instance.c
	src/material.c
	src/matrix.c
	src/mesh.c
	src/obj.c
	src/perm.c
	src/rule.c
	src/shape.c
	src/sim.c
	src/state.c
	src/system.c
	src/vec.c
)

set_target_properties(bfm PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(bfm PROPERTIES SOVERSION 1)

set_target_properties(bfm PROPERTIES PUBLIC_HEADER
	"src/bfm/bfm.h;src/bfm/condition.h;src/bfm/force.h;src/bfm/instance.h;src/bfm/math.h;src/bfm/material.h;src/bfm/matrix.h;src/bfm/mesh.h;src/bfm/obj.h;src/bfm/perm.h;src/bfm/rule.h;src/bfm/shape.h;src/bfm/sim.h;src/bfm/system.h"
)

# CBLAS

find_library(CBLAS_LIBRARY cblas)
find_path(CBLAS_INCLUDE_PATH cblas.h)

if (CBLAS_LIBRARY AND CBLAS_HEADER)
	message("CBLAS found - compiling BFM with CBLAS support")

	target_link_libraries(bfm ${CBLAS_LIBRARY})
	target_compile_definitions(bfm PRIVATE WITH_BLAS)
	target_include_directories(bfm PRIVATE ${CBLAS_INCLUDE_PATH})
endif()

# private include directories

target_include_directories(bfm PRIVATE src)

# install rule
# TODO is this respectful to *BSD systems?

include(GNUInstallDirs)

install(
	TARGETS bfm
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bfm
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
