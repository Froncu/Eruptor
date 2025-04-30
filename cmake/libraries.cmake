find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(SDL_TEST_LIBRARY FALSE)
set(BUILD_SHARED_LIBS FALSE)
FetchContent_Declare(SDL
   GIT_REPOSITORY https://github.com/libsdl-org/SDL
   GIT_TAG release-3.2.8
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

set(SDLIMAGE_AVIF FALSE)
FetchContent_Declare(SDL_image
   GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
   GIT_TAG release-3.2.4
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Declare(glm
   GIT_REPOSITORY https://github.com/g-truc/glm
   GIT_TAG 1.0.1
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Declare(assimp
   GIT_REPOSITORY https://github.com/assimp/assimp
   GIT_TAG v5.4.3
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Declare(spdlog
   GIT_REPOSITORY https://github.com/gabime/spdlog
   GIT_TAG v1.15.2
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_Makeavailable(
   SDL
   SDL_image
   glm
   assimp
   spdlog)