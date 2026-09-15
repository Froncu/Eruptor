#include <eruptor/core/platform.hpp>
#include <eruptor/core/runtime_assert.hpp>
#include <GLFW/glfw3.h>

namespace eru
{
   Platform::Platform(PassKey<Locator>)
   {
      glfwSetErrorCallback(
         []([[maybe_unused]] int const code, [[maybe_unused]] char const* const description)
         {
            RUNTIME_ASSERT(false, std::format("GLFW encountered error code {}! ({})", code, description));
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