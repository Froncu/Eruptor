#ifndef ERUPTOR_HPP
#define ERUPTOR_HPP

#include "context/context_builder.hpp"
#include "renderer/renderer.hpp"
#include "window/window.hpp"

namespace eru
{
   class Eruptor
   {
      public:
         Eruptor() = default;
         Eruptor(Eruptor const&) = delete;
         Eruptor(Eruptor&&) = delete;

         ~Eruptor() = default;

         Eruptor& operator=(Eruptor const&) = delete;
         Eruptor& operator=(Eruptor&&) = delete;

         void run();

      private:
         Context context_{
            ContextBuilder{}
            .enable_validation_layer("VK_LAYER_KHRONOS_validation")
            .build()
         };
         Window window_{ context_ };
         Renderer renderer_{ context_, window_ };
   };
}

#endif