cmake_minimum_required(VERSION 3.29)
project(Eruptor)

add_executable(${PROJECT_NAME}
   source/application/application.cpp
   source/main.cpp
   source/shader_compiler/shader_compiler.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 20
   CXX_STANDARD_REQUIRED ON)
target_compile_options(${PROJECT_NAME} PRIVATE
   $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)
target_link_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-static>)
target_precompile_headers(${PROJECT_NAME} PRIVATE source/erupch.hpp)
target_include_directories(${PROJECT_NAME} PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/source)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(libraries)

# TODO: figure out why Vulkan::shaderc_combined gives linker errors when using MinGW
target_link_libraries(${PROJECT_NAME} PRIVATE
   Vulkan::Vulkan
   $<$<CXX_COMPILER_ID:GNU,Clang>:$ENV{VULKAN_SDK}/Lib/shaderc_shared.lib>
   $<$<CXX_COMPILER_ID:MSVC>:Vulkan::shaderc_combined>
   SDL3::SDL3
   glm::glm-header-only)

add_custom_command(
   TARGET ${PROJECT_NAME} POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
   ${CMAKE_SOURCE_DIR}/resources
   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)