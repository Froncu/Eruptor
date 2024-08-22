#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iostream>

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* const window{ glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr) };

    std::uint32_t extension_count{};
    std::ignore = vk::enumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::cout << extension_count << " extensions supported\n";

    glm::mat4 matrix{};
    glm::vec4 vec{};
    auto test{ matrix * vec };

    while (not glfwWindowShouldClose(window))
        glfwPollEvents();

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}