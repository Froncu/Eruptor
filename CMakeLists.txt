cmake_minimum_required(VERSION 4.4)
project(eruptor LANGUAGES CXX)
set(${PROJECT_NAME}_NAMESPACE eru)

file(GLOB MODULE_SCRIPTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/modules/*/scripts/*.cmake")
foreach (MODULE_SCRIPT IN LISTS MODULE_SCRIPTS)
   include("${MODULE_SCRIPT}")
endforeach ()

file(GLOB MODULE_LISTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/modules/*/CMakeLists.txt")
foreach (MODULE_LIST IN LISTS MODULE_LISTS)
   cmake_path(GET MODULE_LIST PARENT_PATH MODULE_DIRECTORY)
   add_subdirectory("${MODULE_DIRECTORY}")
endforeach ()