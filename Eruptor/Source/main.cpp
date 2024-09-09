#include "erupch.hpp"

#include "Utility/UniquePointer.hpp"

int main()
{
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   eru::UniquePointer<GLFWwindow> const window{
      glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr),
      glfwDestroyWindow
   };

   std::uint32_t extension_count;
   std::ignore = vk::enumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

   std::cout << std::format("{} extensions supported\n", extension_count);

   glm::mat4 matrix{};
   glm::vec4 vec{};
   auto test{ matrix * vec };

   while (not glfwWindowShouldClose(window.get()))
      glfwPollEvents();

   glfwTerminate();

   return 0;
}