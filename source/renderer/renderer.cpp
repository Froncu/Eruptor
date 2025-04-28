#include "renderer.hpp"
#include "window/window.hpp"

namespace eru
{
   Renderer::Renderer(Context const& context, Window const& window)
      : device_{
         DeviceBuilder{ context }
         .add_queues({ vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, window.surface() })
         .build()
      }
   {
   }

   void Renderer::render()
   {
   }
}