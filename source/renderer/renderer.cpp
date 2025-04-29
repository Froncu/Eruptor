#include "renderer.hpp"

namespace eru
{
   Renderer::Renderer(Context const& context, Window const& window)
      : device_{
         DeviceBuilder{}
         .enable_extension(vk::KHRSwapchainExtensionName)
         .add_queues({ vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, window.surface() })
         .build(context)
      }
      , swap_chain_{
         swap_chain_builder_
         .change_format({ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
         .change_present_mode(vk::PresentModeKHR::eMailbox)
         .build(device_, window, { queue_ })
      }
   {
   }

   void Renderer::render()
   {
   }
}