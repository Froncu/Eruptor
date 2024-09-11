#if not defined APPLICATION_HPP
#define APPLICATION_HPP

#include "erupch.hpp"

#include "Utility/UniquePointer.hpp"

namespace eru
{
   class Application final
   {
   public:
      Application();
      Application(Application const&) = delete;
      Application(Application&&) = delete;

      ~Application();

      Application& operator=(Application const&) = delete;
      Application& operator=(Application&&) = delete;

      void run() const;

   private:
      [[nodiscard]] vk::Instance createInstance() const;

      UniquePointer<GLFWwindow> const window_{ glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr), glfwDestroyWindow };
      vk::Instance const instance_{ createInstance() };
   };
}

#endif