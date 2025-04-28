#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "device/device_builder.hpp"

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
         vk::raii::Queue queue_{ device_.queues().front() };
   };
}

#endif