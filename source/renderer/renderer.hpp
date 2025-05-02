#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "device.hpp"
#include "swap_chain.hpp"
#include "window/window.hpp"

namespace eru
{
   class Renderer final
   {
      public:
         explicit Renderer(Window const& window);
         Renderer(Renderer const&) = delete;
         Renderer(Renderer&&) = delete;

         ~Renderer() = default;

         Renderer& operator=(Renderer const&) = delete;
         Renderer& operator=(Renderer&&) = delete;

         void render();

      private:
         Device device_;
         SwapChain swap_chain_;
   };
}

#endif