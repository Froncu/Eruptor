eruptor_module(STATIC)
eruptor_reflect("${MODULE_TARGET_NAME}")

find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)

target_include_directories("${MODULE_TARGET_NAME}"
   PRIVATE "${Stb_INCLUDE_DIR}")

target_compile_definitions("${MODULE_TARGET_NAME}"
   PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE
   PUBLIC VULKAN_HPP_ASSERT_ON_RESULT=void
   PUBLIC VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
   PUBLIC VULKAN_HPP_NO_EXCEPTIONS)

target_link_libraries("${MODULE_TARGET_NAME}"
   PUBLIC glm::glm-header-only
   PUBLIC Vulkan::Headers
   PRIVATE assimp::assimp
   PRIVATE glfw
   PRIVATE KTX::ktx
   PRIVATE slang::slang)