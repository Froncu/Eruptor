find_package(Vulkan REQUIRED)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# TODO: find a way to get rid of the `update_mappings` target
set(GLFW_INSTALL OFF)
FetchContent_Declare(glfw
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG 3.4
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)
FetchContent_Makeavailable(glfw)

FetchContent_Declare(glm
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG 1.0.1
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)
FetchContent_Makeavailable(glm)