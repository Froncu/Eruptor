#include "services/context/context.hpp"
#include "window.hpp"
#include "utility/runtime_assert.hpp"

namespace eru
{
   Window::Window(std::string_view const title, vk::Extent2D const size,
      std::optional<glm::ivec2> const& position, std::uint64_t const flags)
      : native_window_{
         [](std::string_view const title, vk::Extent2D const size, std::uint64_t const flags)
         {
            SDL_Window* const native_window{
               SDL_CreateWindow(title.data(), size.width, size.height, flags | SDL_WINDOW_VULKAN)
            };
            runtime_assert(native_window, "failed to create an SDL window ({})",
               SDL_GetError());

            return native_window;
         }(title, size, flags),
         SDL_DestroyWindow
      }
      , surface_{
         [](SDL_Window& window) -> vk::raii::SurfaceKHR
         {
            vk::raii::Instance const& instance{ Locator::get<Context>()->instance() };

            VkSurfaceKHR surface;
            bool const succeeded{ SDL_Vulkan_CreateSurface(&window, *instance, nullptr, &surface) };
            runtime_assert(succeeded, "failed to create window surface ({})",
               SDL_GetError());

            return { instance, surface };
         }(*native_window_)
      }
   {
      if (position.has_value())
         change_position(*position);
   }

   Window::Window(std::string_view const title, vk::Extent2D const size)
      : Window(title, size, {}, {})
   {
   }

   Window::Window(Window const& other)
      : Window(other.title(), other.extent(), other.position(), SDL_GetWindowFlags(other.native_window_.get()))
   {
   }

   Window& Window::operator=(Window const& other)
   {
      if (this == &other)
         return *this;

      return *this = Window{ other };
   }

   void Window::change_title(std::string_view const title)
   {
      bool const succeeded{ SDL_SetWindowTitle(native_window_.get(), title.data()) };
      runtime_assert(succeeded, "failed to set RenderContext{}'s title to {} ({})",
         id(), title, SDL_GetError());
   }

   void Window::change_extent(vk::Extent2D const size)
   {
      bool const succeeded{
         SDL_SetWindowSize(native_window_.get(), static_cast<int>(size.width), static_cast<int>(size.height))
      };
      runtime_assert(succeeded, "failed to set RenderContext{}'s size to {}x{} ({})",
         id(), size.width, size.height, SDL_GetError());
   }

   void Window::change_position(glm::ivec2 position)
   {
      bool const succeeded{ SDL_SetWindowPosition(native_window_.get(), position.x, position.y) };
      runtime_assert(succeeded, "failed to set RenderContext{}'s position to {}x{} ({})",
         id(), position.x, position.y, SDL_GetError());
   }

   void Window::center()
   {
      change_position({ SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED });
   }

   void Window::change_fullscreen_mode(bool const fullscreen)
   {
      bool const succeeded{ SDL_SetWindowFullscreen(native_window_.get(), fullscreen) };
      runtime_assert(succeeded, "failed to set RenderContext{}'s fullscreen to {} ({})",
         id(), fullscreen ? "fullscreen" : "windowed", SDL_GetError());
   }

   void Window::change_resizability(bool const resizable)
   {
      bool const succeeded{ SDL_SetWindowResizable(native_window_.get(), resizable) };
      runtime_assert(succeeded, "failed to set RenderContext{}'s resizability to {} ({})",
         id(), resizable, SDL_GetError());
   }

   void Window::change_visibility(bool const show)
   {
      bool const succeeded{ show ? SDL_ShowWindow(native_window_.get()) : SDL_HideWindow(native_window_.get()) };
      runtime_assert(succeeded, "failed to set RenderContext{}'s visibility to {} ({})",
         id(), show ? "show" : "hidden", SDL_GetError());
   }

   ID::InternalValue Window::id() const
   {
      ID::InternalValue const id{ SDL_GetWindowID(native_window_.get()) };
      runtime_assert(id, "failed to retrieve the ID of a RenderContext ({})",
         SDL_GetError());

      return id;
   }

   std::string_view Window::title() const
   {
      return SDL_GetWindowTitle(native_window_.get());
   }

   vk::Extent2D Window::extent() const
   {
      int width;
      int height;
      bool const succeeded{ SDL_GetWindowSize(native_window_.get(), &width, &height) };
      runtime_assert(succeeded, "failed to retrieve RenderContext{}'s size ({})",
         id(), SDL_GetError());

      return {
         .width{ static_cast<std::uint32_t>(width) },
         .height{ static_cast<std::uint32_t>(height) }
      };
   }

   glm::ivec2 Window::position() const
   {
      glm::ivec2 position;
      bool const succeeded{ SDL_GetWindowPosition(native_window_.get(), &position.x, &position.y) };
      runtime_assert(succeeded, "failed to retrieve RenderContext{}'s position ({})",
         id(), SDL_GetError());

      return position;
   }

   bool Window::fullscreen() const
   {
      return SDL_GetWindowFlags(native_window_.get()) & SDL_WINDOW_FULLSCREEN;
   }

   bool Window::resizable() const
   {
      return SDL_GetWindowFlags(native_window_.get()) & SDL_WINDOW_RESIZABLE;
   }

   bool Window::visible() const
   {
      return not(SDL_GetWindowFlags(native_window_.get()) & SDL_WINDOW_HIDDEN);
   }

   vk::raii::SurfaceKHR const& Window::surface() const
   {
      return surface_;
   }
}