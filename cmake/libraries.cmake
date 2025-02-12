find_package(Vulkan REQUIRED)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(BUILD_SHARED_LIBS FALSE)
set(SDL_TEST_LIBRARY FALSE)
FetchContent_Declare(SDL
   GIT_REPOSITORY https://github.com/libsdl-org/SDL
   GIT_TAG release-3.2.4
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)
FetchContent_Makeavailable(SDL)

FetchContent_Declare(glm
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG 1.0.1
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)
FetchContent_Makeavailable(glm)