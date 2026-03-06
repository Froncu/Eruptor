#include "eruptor/application.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"
#include "eruptor/runtime_assert.hpp"
#include "eruptor/window.hpp"

namespace eru
{
   Application::GLFWcontext::GLFWcontext()
   {
      glfwSetErrorCallback(
         [](int const code, char const* const description)
         {
            runtime_assert(false, std::format("GLFW encountered error code {}! ({})", code, description));
         });

      glfwInit();
   }

   Application::GLFWcontext::~GLFWcontext()
   {
      glfwTerminate();
   }

   Application::~Application()
   {
      Locator::remove_all();
   }

   void Application::poll()
   {
      glfwPollEvents();
   }

   Application::Application()
   {
      Locator::provide<Application>(*this);
      Locator::provide<Logger>();
      Locator::provide<Window>();
   }
}