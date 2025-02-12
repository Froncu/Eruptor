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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(libraries)

target_include_directories(${PROJECT_NAME} PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/source
   ${Vulkan_INCLUDE_DIRS}
   ${glm_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
   ${Vulkan_LIBRARIES}
   $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib
   glfw)

add_custom_command(
   TARGET ${PROJECT_NAME} POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
   ${CMAKE_SOURCE_DIR}/resources
   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)