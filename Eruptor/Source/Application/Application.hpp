#if not defined APPLICATION_HPP
#define APPLICATION_HPP

#include "erupch.hpp"

#include "utility/unique_pointer.hpp"

namespace eru
{
   class application final
   {
   public:
      application();
      application(application const&) = delete;
      application(application&&) = delete;

      ~application();

      application& operator=(application const&) = delete;
      application& operator=(application&&) = delete;

      void run() const;

   private:
      [[nodiscard]] vk::Instance create_instance() const;

      unique_pointer<GLFWwindow> const window_{ glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr), glfwDestroyWindow };
      vk::Instance const instance_{ create_instance() };
   };
}

#endif