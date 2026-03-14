#include "eruptor/window.hpp"

#include "core/dependencies.hpp"

namespace eru
{
   Window::Window(glm::uvec2 const extent, std::string_view const title)
      : native_window_{
         glfwCreateWindow(extent.x, extent.y, title.data(), nullptr, nullptr),
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