cmake_minimum_required(VERSION 3.29)
project(Eruptor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS --static)

# Find Vulkan
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

include(FetchContent)

# Fetch GLFW
fetchcontent_declare(
   glfw
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG 3.4
)
fetchcontent_makeavailable(glfw)

# Fetch glm
fetchcontent_declare(
   glm
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG 1.0.1
)
fetchcontent_getproperties(glm)
if(NOT glm_POPULATED)
   fetchcontent_populate(glm)
endif()

add_executable(${PROJECT_NAME}
   source/application/application.cpp
   source/main.cpp
   source/shader_compiler/shader_compiler.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE source/erupch.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/source
   ${glm_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
   ${Vulkan_LIBRARIES}
   $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib
   glfw
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})