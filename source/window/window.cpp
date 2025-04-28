#include "window.hpp"

namespace eru
{
   Window::Window(Context const& context, std::string_view const title, vk::Extent2D const extent)
      : native_window_{
         [title, extent]
         {
            if (not SDL_Init(SDL_INIT_VIDEO))
               throw std::runtime_error(std::format("failed to initialize the video subsystem ({})", SDL_GetError()));

            return SDL_CreateWindow(title.data(), static_cast<int>(extent.width), static_cast<int>(extent.height),
               SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
         }(),
         [](SDL_Window* const window)
         {
            SDL_DestroyWindow(window);
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
         }
      }
      , surface_{
         context.instance_,
         [this, &context]
         {
            if (VkSurfaceKHR surface; SDL_Vulkan_CreateSurface(native_window_.get(),
               *context.instance_, nullptr, &surface))
               return surface;

            throw std::runtime_error(std::format("failed to create window surface ({})", SDL_GetError()));
         }()
      }
   {
   }

   SDL_Window* Window::native_window() const
   {
      return native_window_.get();
   }

   vk::raii::SurfaceKHR const& Window::surface() const
   {
      return surface_;
   }
}