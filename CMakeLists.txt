cmake_minimum_required(VERSION 3.30)
project(eruptor LANGUAGES CXX)

file(GLOB MODULE_LISTS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/modules/*/CMakeLists.txt")
foreach (MODULE_LIST IN LISTS MODULE_LISTS)
   cmake_path(GET MODULE_LIST PARENT_PATH MODULE_DIRECTORY)
   add_subdirectory("${MODULE_DIRECTORY}")
endforeach ()