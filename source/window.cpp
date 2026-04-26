#include "eruptor/application.hpp"
#include "eruptor/locator.hpp"
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
      glfwSetWindowCloseCallback(native_window_.get(),
         [](GLFWwindow* const)
         {
            Locator::get<Application>().keep_ticking = false;
         });
   }

   auto Window::native() const -> GLFWwindow&
   {
      return *native_window_;
   }

   auto Window::change_visibility(bool const visible) -> void
   {
      visible
         ? glfwShowWindow(native_window_.get())
         : glfwHideWindow(native_window_.get());
   }

   auto Window::change_extent(glm::uvec2 const extent) -> void
   {
      glfwSetWindowSize(native_window_.get(), extent.x, extent.y);
   }

   auto Window::extent() const -> glm::uvec2
   {
      glm::ivec2 extent;
      glfwGetWindowSize(native_window_.get(), &extent.x, &extent.y);
      return extent;
   }

   auto Window::change_position(glm::uvec2 const position) -> void
   {
      glfwSetWindowPos(native_window_.get(), position.x, position.y);
   }

   auto Window::position() const -> glm::uvec2
   {
      glm::ivec2 position;
      glfwGetWindowPos(native_window_.get(), &position.x, &position.y);
      return position;
   }

   auto Window::change_title(std::string_view const title) -> void
   {
      glfwSetWindowTitle(native_window_.get(), title.data());
   }

   auto Window::title() const -> std::string_view
   {
      return glfwGetWindowTitle(native_window_.get());
   }
}