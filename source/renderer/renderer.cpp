#include "renderer.hpp"
#include "render_pass/render_pass_builder.hpp"

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
      , render_pass_{
         RenderPassBuilder{}
         .add_attachments({
            {
               {
                  .format{ swap_chain_.images().front().format() },
                  .samples{ vk::SampleCountFlagBits::e1 },
                  .loadOp{ vk::AttachmentLoadOp::eClear },
                  .storeOp{ vk::AttachmentStoreOp::eStore },
                  .stencilLoadOp{ vk::AttachmentLoadOp::eDontCare },
                  .stencilStoreOp{ vk::AttachmentStoreOp::eDontCare },
                  .initialLayout{ vk::ImageLayout::eUndefined },
                  .finalLayout{ vk::ImageLayout::ePresentSrcKHR }
               },
               {
                  .format{ vk::Format::eD32Sfloat },
                  .samples{ vk::SampleCountFlagBits::e1 },
                  .loadOp{ vk::AttachmentLoadOp::eClear },
                  .storeOp{ vk::AttachmentStoreOp::eDontCare },
                  .stencilLoadOp{ vk::AttachmentLoadOp::eDontCare },
                  .stencilStoreOp{ vk::AttachmentStoreOp::eDontCare },
                  .initialLayout{ vk::ImageLayout::eUndefined },
                  .finalLayout{ vk::ImageLayout::eDepthStencilAttachmentOptimal }
               }
            }
         })
         .add_subpass({
            .pipeline_bind_point{ vk::PipelineBindPoint::eGraphics },
            .color_resolve_attachment_references{
               {
                  {
                     .attachment{ 0 },
                     .layout{ vk::ImageLayout::eColorAttachmentOptimal },
                  }
               }
            },
            .depth_stencil_attachment_reference{
               .attachment{ 1 },
               .layout{ vk::ImageLayout::eDepthStencilAttachmentOptimal }
            }
         })
         .add_subpass_dependency({
            .srcSubpass{ vk::SubpassExternal },
            .dstSubpass{ 0 },
            .srcStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests },
            .dstStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests },
            .srcAccessMask{ vk::AccessFlagBits::eNone },
            .dstAccessMask{ vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite }
         })
         .build(device_)
      }
   {
   }

   void Renderer::render()
   {
   }
}