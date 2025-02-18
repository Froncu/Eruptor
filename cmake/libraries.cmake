find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(BUILD_SHARED_LIBS FALSE)
set(SDL_TEST_LIBRARY FALSE)
FetchContent_Declare(SDL
   GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
   GIT_TAG release-3.2.4
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Declare(glm
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG 1.0.1
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Declare(imgui
   GIT_REPOSITORY https://github.com/ocornut/imgui.git
   GIT_TAG v1.91.8
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Makeavailable(
   SDL
   glm
   imgui)

add_library(imgui STATIC
   ${imgui_SOURCE_DIR}/imgui.cpp
   ${imgui_SOURCE_DIR}/imgui_demo.cpp
   ${imgui_SOURCE_DIR}/imgui_draw.cpp
   ${imgui_SOURCE_DIR}/imgui_tables.cpp
   ${imgui_SOURCE_DIR}/imgui_widgets.cpp
   ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
   ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

target_link_libraries(imgui
   PUBLIC Vulkan::Vulkan
   PUBLIC SDL3::SDL3)