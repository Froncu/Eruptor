cmake_minimum_required(VERSION 3.15)
project(Eruptor)

file(GLOB_RECURSE ERU_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${ERU_SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 20
   CXX_STANDARD_REQUIRED TRUE)

target_compile_options(${PROJECT_NAME}
   PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
   PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)

target_precompile_headers(${PROJECT_NAME}
   PRIVATE source/erupch/erupch.hpp)

target_include_directories(${PROJECT_NAME}
   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(libraries)

target_link_libraries(${PROJECT_NAME}
   PRIVATE Vulkan::Vulkan
   PRIVATE Vulkan::shaderc_combined
   PRIVATE SDL3::SDL3
   PRIVATE SDL3_image::SDL3_image
   PRIVATE glm::glm-header-only
   PRIVATE assimp::assimp
   PRIVATE spdlog::spdlog)

add_custom_command(POST_BUILD TARGET ${PROJECT_NAME}
   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
   ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)