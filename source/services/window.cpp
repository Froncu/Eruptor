#include <glfw/glfw3.h>

#include "eruptor/window.hpp"

namespace eru
{
   Window::Window()
      : native_window_{
         []
         {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            GLFWwindow* const window{ glfwCreateWindow(640, 480, "", nullptr, nullptr) };

            return window;
         }(),
         glfwDestroyWindow
      }
   {
   }

   void Window::change_visibility(bool const visible)
   {
      visible
         ? glfwShowWindow(native_window_.get())
         : glfwHideWindow(native_window_.get());
   }

   void Window::change_extent(glm::uvec2 const extent)
   {
      glfwSetWindowSize(native_window_.get(), extent.x, extent.y);
   }

   glm::uvec2 Window::extent() const
   {
      glm::ivec2 extent;
      glfwGetWindowSize(native_window_.get(), &extent.x, &extent.y);
      return extent;
   }

   void Window::change_position(glm::uvec2 const position)
   {
      glfwSetWindowPos(native_window_.get(), position.x, position.y);
   }

   glm::uvec2 Window::position() const
   {
      glm::ivec2 position;
      glfwGetWindowPos(native_window_.get(), &position.x, &position.y);
      return position;
   }

   void Window::change_title(std::string_view const title)
   {
      glfwSetWindowTitle(native_window_.get(), title.data());
   }

   std::string_view Window::title() const
   {
      return glfwGetWindowTitle(native_window_.get());
   }
}