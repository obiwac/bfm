cmake_minimum_required(VERSION 3.9)
project(libbfm VERSION 0.1.0 DESCRIPTION "Big F'ing Matrix")

# library setup

add_library(libbfm SHARED
	src/matrix.c
	src/mesh.c
	src/parse.c
	src/state.c
)

set_target_properties(libbfm PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(libbfm PROPERTIES SOVERSION 1)
set_target_properties(libbfm PROPERTIES PUBLIC_HEADER src/bfm/bfm.h)

# private include directories

target_include_directories(libbfm PRIVATE src/bfm)

# install rule
# TODO are public headers installed to bfm/bfm.h or bfm.h?
# TODO is this respectful to *BSD systems?

include(GNUInstallDirs)

install(TARGETS libbfm
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
