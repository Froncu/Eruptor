#include "depth_pass.hpp"
#include "erupch/erupch.hpp"
#include "scene/vertex.hpp"

namespace eru
{
   DepthPass::DepthPass(Device const& device, vk::Extent2D const swap_chain_extent, DescriptorSets const& descriptor_sets)
      : swap_chain_extent_{ swap_chain_extent }
      , descriptor_sets_{ descriptor_sets }
      , vertex_shader_{ "resources/shaders/depth.vert", device }
      , pipeline_{
         PipelineBuilder{}
         .change_depth_attachment_format(vk::Format::eD32Sfloat)
         .add_vertex_bindings(Vertex::BINDING_DESCRIPTIONS)
         .add_vertex_attribute(Vertex::ATTRIBUTE_DESCRIPTIONS[0])
         .add_shader_stages({
            {
               .stage{ vk::ShaderStageFlagBits::eVertex },
               .module{ *vertex_shader_.module() },
               .pName{ "main" }
            }
         })
         .assign_descriptor_set_layout("camera", 0)
         .change_depth_stencil_state({
            .depthTestEnable{ true },
            .depthWriteEnable{ true },
            .depthCompareOp{ vk::CompareOp::eLessOrEqual }
         })
         .build(device, descriptor_sets)
      }
      , depth_image_builder_{
         ImageBuilder{}
         .change_type(vk::ImageType::e2D)
         .change_format(vk::Format::eD32Sfloat)
         .change_extent(swap_chain_extent)
         .change_mip_levels(1)
         .change_array_layers(1)
         .change_samples(vk::SampleCountFlagBits::e1)
         .change_tiling(vk::ImageTiling::eOptimal)
         .change_usage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
         .change_sharing_mode(vk::SharingMode::eExclusive)
         .change_initial_layout(vk::ImageLayout::eUndefined)
         .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
      }
      , depth_image_{ depth_image_builder_.build(device) }
      , depth_image_view_{ depth_image_view_builder_.build(device, depth_image_) }
   {
      depth_image_.transition_layout(device, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
   }

   ImageView const& DepthPass::depth_image_view() const
   {
      return depth_image_view_;
   }

   void DepthPass::render(vk::raii::CommandBuffer const& command_buffer, Scene const& scene, std::uint32_t current_frame) const
   {
      vk::ImageMemoryBarrier2 const begin_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eEarlyFragmentTests },
         .dstAccessMask{ vk::AccessFlagBits2::eDepthStencilAttachmentWrite },
         .oldLayout{ vk::ImageLayout::eDepthStencilReadOnlyOptimal },
         .newLayout{ vk::ImageLayout::eDepthStencilAttachmentOptimal },
         .image{ depth_image_.image() },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eDepth },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &begin_barrier }
      });

      vk::RenderingAttachmentInfo const depth_attachment_info{
         .imageView{ *depth_image_view_.image_view() },
         .imageLayout{ vk::ImageLayout::eDepthStencilAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .clearValue{
            vk::ClearDepthStencilValue{
               .depth{ 1.0f }
            }
         },
      };

      command_buffer.beginRendering({
         .renderArea{
            .extent{ swap_chain_extent_ }
         },
         .layerCount{ 1 },
         .pDepthAttachment{ &depth_attachment_info }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.pipeline());

      command_buffer.setViewport(0, {
         {
            .width{ static_cast<float>(swap_chain_extent_.width) },
            .height{ static_cast<float>(swap_chain_extent_.height) },
            .maxDepth{ 1.0f }
         }
      });

      command_buffer.setScissor(0, {
         {
            .extent{ swap_chain_extent_ }
         }
      });

      command_buffer.bindVertexBuffers(0, { scene.vertex_buffer().buffer() }, { {} });
      command_buffer.bindIndexBuffer(scene.index_buffer().buffer(), 0, vk::IndexType::eUint32);
      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.layout(), 0,
         {
            descriptor_sets_.sets("camera")[current_frame]
         }, {});

      for (auto const& [vertex_offset, index_offset, index_count, material_index] : scene.sub_meshes())
         command_buffer.drawIndexed(
            index_count,
            1,
            index_offset,
            vertex_offset,
            0);

      command_buffer.endRendering();

      vk::ImageMemoryBarrier2 const end_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eEarlyFragmentTests },
         .srcAccessMask{ vk::AccessFlagBits2::eDepthStencilAttachmentWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eEarlyFragmentTests },
         .dstAccessMask{ vk::AccessFlagBits2::eDepthStencilAttachmentRead },
         .oldLayout{ vk::ImageLayout::eDepthStencilAttachmentOptimal },
         .newLayout{ vk::ImageLayout::eDepthStencilReadOnlyOptimal },
         .image{ depth_image_.image() },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eDepth },
            .levelCount{ 1 },
            .layerCount{ 1 },
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &end_barrier }
      });
   }

   void DepthPass::recreate_depth_image(Device const& device, vk::Extent2D const swap_chain_extent)
   {
      depth_image_builder_.change_extent(swap_chain_extent);
      depth_image_ = depth_image_builder_.build(device);
      depth_image_.transition_layout(device, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
      depth_image_view_ = depth_image_view_builder_.build(device, depth_image_);
      swap_chain_extent_ = swap_chain_extent;
   }
}