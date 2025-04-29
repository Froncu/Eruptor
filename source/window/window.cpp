#include "window.hpp"

namespace eru
{
   Window::Window(Context const& context, std::string_view const title, vk::Extent2D const extent)
      : window_{
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
         context.instance(),
         [this, &context]
         {
            if (VkSurfaceKHR surface; SDL_Vulkan_CreateSurface(window_.get(),
               *context.instance(), nullptr, &surface))
               return surface;

            throw std::runtime_error(std::format("failed to create window surface ({})", SDL_GetError()));
         }()
      }
   {
   }

   SDL_Window* Window::window() const
   {
      return window_.get();
   }

   vk::raii::SurfaceKHR const& Window::surface() const
   {
      return surface_;
   }

   vk::Extent2D Window::extent() const
   {
      int width;
      int height;
      if (not SDL_GetWindowSizeInPixels(window_.get(), &width, &height))
         throw std::runtime_error(std::format("failed to get window size in pixels: {}\n", SDL_GetError()));

      return { static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height) };
   }
}