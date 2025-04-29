#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "context/context.hpp"
#include "erupch/erupch.hpp"
#include "utility/unique_pointer.hpp"

namespace eru
{
   class Window
   {
      public:
         explicit Window(Context const& context, std::string_view title = "Eruptor", vk::Extent2D extent = { 1280, 720 });
         Window(Window const&) = delete;
         Window(Window&&) = delete;

         ~Window() = default;

         Window& operator=(Window const&) = delete;
         Window& operator=(Window&&) = delete;

         [[nodiscard]] SDL_Window* window() const;
         [[nodiscard]] vk::raii::SurfaceKHR const& surface() const;
         [[nodiscard]] vk::Extent2D extent() const;

      private:
         UniquePointer<SDL_Window> const window_;
         vk::raii::SurfaceKHR const surface_;
   };
}

#endif