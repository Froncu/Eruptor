#include "eruptor/runtime_assert.hpp"
#include "eruptor/window.hpp"

namespace eru
{
   Window::Window()
      : native_window_{
         SDL_CreateWindow("", 640, 480, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN),
         SDL_DestroyWindow
      }
   {
      runtime_assert(native_window_.get(), std::format("failed to create window! ({})", SDL_GetError()));

      try
      {
         bool const result{ SDL_SetWindowParent(native_window_.get(), Locator::get<Window>().native_window_.get()) };
         runtime_assert(result, std::format("failed to set window's parent! ({})", SDL_GetError()));
      }
      catch (...)
      {
      }
   }

   void Window::change_visibility(bool const visible)
   {
      bool const result{ visible ? SDL_ShowWindow(native_window_.get()) : SDL_HideWindow(native_window_.get()) };
      runtime_assert(result, std::format("failed to {} window! ({})", visible ? "show" : "hide", SDL_GetError()));
   }

   void Window::change_extent(glm::uvec2 const extent)
   {
      bool const result{ SDL_SetWindowSize(native_window_.get(), extent.x, extent.y) };
      runtime_assert(result, std::format("failed to change window extent! ({})", SDL_GetError()));
   }

   glm::uvec2 Window::extent() const
   {
      glm::ivec2 extent;
      bool const result{ SDL_GetWindowSize(native_window_.get(), &extent.x, &extent.y) };
      runtime_assert(result, std::format("failed to query window extent! ({})", SDL_GetError()));

      return extent;
   }

   void Window::change_position(glm::uvec2 const position)
   {
      bool const result{ SDL_SetWindowPosition(native_window_.get(), position.x, position.y) };
      runtime_assert(result, std::format("failed to change window position! ({})", SDL_GetError()));
   }

   glm::uvec2 Window::position() const
   {
      glm::ivec2 position;
      bool const result{ SDL_GetWindowPosition(native_window_.get(), &position.x, &position.y) };
      runtime_assert(result, std::format("failed to query window position! ({})", SDL_GetError()));

      return position;
   }

   void Window::change_title(std::string_view const title)
   {
      bool const result{ SDL_SetWindowTitle(native_window_.get(), title.data()) };
      runtime_assert(result, std::format("failed to change window title! ({})", SDL_GetError()));
   }

   std::string_view Window::title() const
   {
      return SDL_GetWindowTitle(native_window_.get());
   }
}