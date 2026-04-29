#include "eruptor/platform.hpp"
#include "eruptor/runtime_assert.hpp"

#include "core/dependencies.hpp"

namespace eru
{
   Platform::Platform(Locator::ConstructionKey)
   {
      glfwSetErrorCallback(
         [](int const code, char const* const description)
         {
            runtime_assert(false, std::format("GLFW encountered error code {}! ({})", code, description));
         });

      glfwInit();
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
   }

   Platform::~Platform()
   {
      glfwTerminate();
   }

   auto Platform::poll() const -> void
   {
      glfwPollEvents();
   }
}