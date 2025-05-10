#include "builders/device_builder.hpp"
#include "builders/swap_chain_builder.hpp"
#include "builders/pipeline_builder.hpp"
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
         .add_queues({ vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, window.surface() })
         .build()
      }
      , swap_chain_{
         SwapChainBuilder{}
         .change_format({ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
         .change_present_mode(vk::PresentModeKHR::eMailbox)
         .build(device_, window, device_.queues())
      }
      , pipeline_{
         PipelineBuilder{}
         .change_color_attachment_format(swap_chain_.images().front().format())
         .add_shader_stages({
            {
               .stage{ vk::ShaderStageFlagBits::eVertex },
               .module{ *vertex_shader_.module() },
               .pName{ "main" }
            },
            {
               .stage{ vk::ShaderStageFlagBits::eFragment },
               .module{ *fragment_shader_.module() },
               .pName{ "main" }
            }
         })
         .add_descriptor_bindings({
            {
               .type{ vk::DescriptorType::eUniformBuffer },
               .shader_stage_flags{ vk::ShaderStageFlagBits::eVertex },
               .count{ 1 }
            },
            {
               .type{ vk::DescriptorType::eCombinedImageSampler },
               .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
               .count{ 1 }
            }
         })
         .build(device_)
      }
   {
   }

   void Renderer::render()
   {
   }
}