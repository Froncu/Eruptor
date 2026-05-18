#include "eruptor/platform.hpp"
#include "eruptor/runtime_assert.hpp"

#include "core/dependencies.hpp"

namespace eru
{
   Platform::Platform(Locator::ConstructionKey)
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