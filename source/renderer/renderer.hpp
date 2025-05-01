#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "builders/device_builder.hpp"
#include "window/window.hpp"

namespace eru
{
   class Renderer final
   {
      public:
         Renderer(Context const& context, Window const& window);
         Renderer(Renderer const&) = delete;
         Renderer(Renderer&&) = delete;

         ~Renderer() = default;

         Renderer& operator=(Renderer const&) = delete;
         Renderer& operator=(Renderer&&) = delete;

         void render();
   };
}

#endif