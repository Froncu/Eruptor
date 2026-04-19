cmake_minimum_required(VERSION 3.15)
project(eruptor)

option(ERU_BUILD_SHARED "Build ${PROJECT_NAME} as shared library" ${BUILD_SHARED_LIBS})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(project_defaults)
include(umbrella_header)

find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)

# Framework library - contains all functionality
# Can be built as SHARED or STATIC depending on ERU_BUILD_SHARED
# Direct linking is possible but not recommended; use the entry point library instead unless you know what you are doing

file(GLOB_RECURSE ERU_SOURCE_FILES CONFIGURE_DEPENDS source/*.cpp)
list(FILTER ERU_SOURCE_FILES EXCLUDE REGEX entry\\.cpp)
if(ERU_BUILD_SHARED)
   add_library(${PROJECT_NAME} SHARED ${ERU_SOURCE_FILES})
else()
   add_library(${PROJECT_NAME} STATIC ${ERU_SOURCE_FILES})
endif()

target_include_directories(${PROJECT_NAME}
   PUBLIC include
   PRIVATE source)

target_precompile_headers(${PROJECT_NAME}
   PUBLIC include/${PROJECT_NAME}/pch.hpp
   PRIVATE source/core/dependencies.hpp)

target_compile_definitions(${PROJECT_NAME}
   PUBLIC VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
   PUBLIC VULKAN_HPP_NO_EXCEPTIONS
   PUBLIC $<$<BOOL:${ERU_BUILD_SHARED}>:ERU_SHARED>
   PRIVATE ERU_FRAMEWORK_LEVEL
   PRIVATE ERU_COMPILE_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(${PROJECT_NAME}
   PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-fvisibility=hidden -fvisibility-inlines-hidden>)

target_link_libraries(${PROJECT_NAME}
   PUBLIC glm::glm-header-only
   PUBLIC Vulkan::Headers
   PRIVATE assimp::assimp
   PRIVATE glfw
   PRIVATE KTX::ktx
   PRIVATE slang::slang)

set_project_defaults(${PROJECT_NAME})
generate_umbrella_header(${PROJECT_NAME}
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME})

# Entry point library - provides the application entry point
# This is the recommended target for executables to link against
# Automatically includes the framework library as a transitive dependency

set(ERU_ENTRY_NAME ${PROJECT_NAME}_entry)

add_library(${ERU_ENTRY_NAME} STATIC
   source/core/entry.cpp)

target_link_libraries(${ERU_ENTRY_NAME}
   PUBLIC ${PROJECT_NAME})

set_project_defaults(${ERU_ENTRY_NAME})