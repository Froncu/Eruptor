include(eruptor_module)
eruptor_module(STATIC)

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
   PUBLIC VULKAN_HPP_NO_EXCEPTIONS
   PRIVATE ERU_FRAMEWORK_LEVEL
   PRIVATE "ERU_COMPILE_SOURCE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

target_link_libraries("${MODULE_TARGET_NAME}"
   PUBLIC glm::glm-header-only
   PUBLIC Vulkan::Headers
   PRIVATE assimp::assimp
   PRIVATE glfw
   PRIVATE KTX::ktx
   PRIVATE slang::slang)

set_property(TARGET "${MODULE_TARGET_NAME}" PROPERTY EXPORT_COMPILE_COMMANDS ON)
add_custom_command(TARGET "${MODULE_TARGET_NAME}" PRE_LINK
   COMMAND "$<TARGET_FILE:eru::reflection>"
   WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
   COMMENT "Running Eruptor reflection" VERBATIM)