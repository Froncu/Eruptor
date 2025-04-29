#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "device/device_builder.hpp"
#include "swap_chain/swap_chain_builder.hpp"
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

      private:
         Device device_;
         DeviceQueue const& queue_{ device_.queues().front() };
         SwapChainBuilder swap_chain_builder_{};
         SwapChain swap_chain_;
   };
}

#endif