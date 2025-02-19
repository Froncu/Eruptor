cmake_minimum_required(VERSION 3.15)
project(Eruptor)

# TODO: does this check if we are on Windows or macOS?
add_executable(${PROJECT_NAME}
   source/application/application.cpp
   source/main.cpp
   source/shader_compiler/shader_compiler.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 20
   CXX_STANDARD_REQUIRED TRUE)
target_compile_options(${PROJECT_NAME}
   PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
   PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)
# TODO: figure out why the following is needed for MinGW
target_link_options(${PROJECT_NAME}
   PRIVATE $<$<BOOL:MINGW>:-static-libgcc -static-libstdc++>)
target_precompile_headers(${PROJECT_NAME}
   PRIVATE source/erupch.hpp)
target_include_directories(${PROJECT_NAME}
   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(libraries)

# TODO: figure out why Vulkan::shaderc_combined gives linker errors when using MinGW
if (MINGW)
   set(shaderc_lib $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib)
else ()
   set(shaderc_lib Vulkan::shaderc_combined)
endif ()

target_link_libraries(${PROJECT_NAME}
   PRIVATE Vulkan::Vulkan
   PRIVATE ${shaderc_lib}
   PRIVATE SDL3::SDL3
   PRIVATE glm::glm-header-only)

add_custom_command(TARGET POST_BUILD ${PROJECT_NAME}
   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
   ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)