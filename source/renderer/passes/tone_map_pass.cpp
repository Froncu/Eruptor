#include "tone_map_pass.hpp"
#include "builders/graphics_pipeline_builder.hpp"

namespace eru
{
   ToneMapPass::ToneMapPass(Device const& device, vk::Format swap_chain_format, DescriptorSets const& descriptor_sets)
      : descriptor_sets_{ descriptor_sets }
      , vertex_shader_{ "resources/shaders/fullscreen_quad.vert", device }
      , fragment_shader_{ "resources/shaders/tone_mapping.frag", device }
      , pipeline_{
         GraphicsPipelineBuilder{}
         .add_color_attachment_format(swap_chain_format)
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
         .change_input_assembly_state({
            .topology{ vk::PrimitiveTopology::eTriangleStrip }
         })
         .assign_descriptor_set_layout("hdr", 0)
         .change_depth_stencil_state({})
         .add_push_constant_range({
            .stageFlags{ vk::ShaderStageFlagBits::eFragment },
            .offset{ 0 },
            .size{ sizeof(std::uint32_t) }
         })
         .build(device, descriptor_sets_)
      }
   {
   }

   void ToneMapPass::render(vk::raii::CommandBuffer const& command_buffer, Image const& swap_chain_image,
      ImageView const& swap_chain_image_view, vk::Extent2D const swap_chain_extent, std::uint32_t const current_frame) const
   {
      vk::ImageMemoryBarrier2 const color_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .oldLayout{ vk::ImageLayout::eUndefined },
         .newLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .image{ swap_chain_image.image() },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &color_barrier }
      });

      vk::RenderingAttachmentInfo const color_attachment_info{
         .imageView{ *swap_chain_image_view.image_view() },
         .imageLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .clearValue{
            vk::ClearColorValue{
               std::array{ 0.0f, 0.0f, 0.0f, 1.0f }
            }
         }
      };

      command_buffer.beginRendering({
         .renderArea{
            .extent{ swap_chain_extent }
         },
         .layerCount{ 1 },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &color_attachment_info }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.pipeline());

      command_buffer.setViewport(0, {
         {
            .width{ static_cast<float>(swap_chain_extent.width) },
            .height{ static_cast<float>(swap_chain_extent.height) },
            .maxDepth{ 1.0f }
         }
      });

      command_buffer.setScissor(0, {
         {
            .extent{ swap_chain_extent }
         }
      });

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.layout(), 0,
         {
            descriptor_sets_.sets("hdr").front()
         }, {});

      command_buffer.pushConstants<std::uint32_t>(*pipeline_.layout(),
         vk::ShaderStageFlagBits::eFragment, 0, {
            current_frame
         });

      command_buffer.draw(4, 1, 0, 0);

      command_buffer.endRendering();

      vk::ImageMemoryBarrier2 const present_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .srcAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eBottomOfPipe },
         .dstAccessMask{ vk::AccessFlagBits2::eNone },
         .oldLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .newLayout{ vk::ImageLayout::ePresentSrcKHR },
         .image{ swap_chain_image.image() },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &present_barrier }
      });
   }
}