#include "builders/device_builder.hpp"
#include "builders/swap_chain_builder.hpp"
#include "renderer.hpp"

namespace eru
{
   Renderer::Renderer(Window const& window)
      : device_{
         DeviceBuilder{}
         .enable_extensions({
            vk::KHRSwapchainExtensionName,
            vk::KHRSynchronization2ExtensionName,
            vk::KHRDynamicRenderingExtensionName
         })
         .enable_features({ .samplerAnisotropy{ true } })
         .add_queues({ vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, window.surface() }, 1)
         .build()
      }
      , swap_chain_{
         SwapChainBuilder{}
         .change_format({ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
         .change_present_mode(vk::PresentModeKHR::eMailbox)
         .change_old_swap_chain(&swap_chain_)
         .build(device_, window, device_.queues())
      }
   {
   }

   void Renderer::render()
   {
   }
}