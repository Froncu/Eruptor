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

target_precompile_headers(${PROJECT_NAME} PRIVATE source/erupch.hpp)

# Find Vulkan
find_package(Vulkan REQUIRED)

include(FetchContent)

# Fetch GLFW
fetchcontent_declare(glfw
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG 3.4
)
fetchcontent_makeavailable(glfw)

# Fetch glm
fetchcontent_declare(glm
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG 1.0.1
)
fetchcontent_makeavailable(glm)
#if(NOT glm_POPULATED)
#   fetchcontent_populate(glm)
#endif()

target_include_directories(${PROJECT_NAME} PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/source
   ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
   ${Vulkan_LIBRARIES}
   $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib
   glfw
   glm)

add_custom_command(
   TARGET ${PROJECT_NAME} POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
   ${CMAKE_SOURCE_DIR}/resources
   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)