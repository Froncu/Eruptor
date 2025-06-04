#include "renderer.hpp"

namespace eru
{
   Renderer::Renderer(Window const& window)
      : window_{ window }
   {
      camera.change_projection_extent(swap_chain_.extent());
   }

   Renderer::~Renderer()
   {
      device_.device().waitIdle();
   }

   void Renderer::render()
   {
      if (window_->minimised())
         return;

      vk::raii::Fence const& command_buffer_executed_fence{ command_buffer_executed_fences_[current_frame_] };
      vk::raii::Semaphore const& image_available_semaphore{ image_available_semaphores_[current_frame_] };
      vk::raii::Semaphore const& render_finished_semaphore{ render_finished_semaphores_[current_frame_] };
      vk::raii::CommandBuffer const& command_buffer{ command_buffers_[current_frame_] };

      if (device_.device().waitForFences({ command_buffer_executed_fence }, true,
         std::numeric_limits<std::uint64_t>::max()) not_eq vk::Result::eSuccess)
         exception("failed to wait for fences!");

      camera_buffers_[current_frame_].upload(&camera.data(), sizeof(camera.data()));

      auto&& [result, image_index]{
         device_.device().acquireNextImage2KHR({
            .swapchain{ *swap_chain_.swap_chain() },
            .timeout{ std::numeric_limits<std::uint64_t>::max() },
            .semaphore{ *image_available_semaphore },
            .deviceMask{ 1 }
         })
      };

      Image const& image{ swap_chain_.images()[image_index] };
      ImageView const& image_view{ swap_chain_.image_views()[image_index] };

      device_.device().resetFences({ command_buffer_executed_fence });

      command_buffer.reset();

      command_buffer.begin({});

      depth_pass_.render(command_buffer, scene_, current_frame_);

      vk::ImageMemoryBarrier2 const color_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .oldLayout{ vk::ImageLayout::eUndefined },
         .newLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .image{ image.image() },
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
         .imageView{ *image_view.image_view() },
         .imageLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .clearValue{
            vk::ClearColorValue{
               std::array{ 0.0f, 0.0f, 0.0f, 1.0f }
            }
         }
      };

      vk::RenderingAttachmentInfo const depth_attachment_info{
         .imageView{ *depth_pass_.depth_image_view().image_view() },
         .imageLayout{ vk::ImageLayout::eDepthStencilReadOnlyOptimal },
         .loadOp{ vk::AttachmentLoadOp::eLoad },
         .storeOp{ vk::AttachmentStoreOp::eDontCare }
      };

      command_buffer.beginRendering({
         .renderArea{
            .extent{ swap_chain_.extent() }
         },
         .layerCount{ 1 },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &color_attachment_info },
         .pDepthAttachment{ &depth_attachment_info }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.pipeline());

      command_buffer.setViewport(0, {
         {
            .width{ static_cast<float>(swap_chain_.extent().width) },
            .height{ static_cast<float>(swap_chain_.extent().height) },
            .maxDepth{ 1.0f }
         }
      });

      command_buffer.setScissor(0, {
         {
            .extent{ swap_chain_.extent() }
         }
      });

      command_buffer.bindVertexBuffers(0, { scene_.vertex_buffer().buffer() }, { {} });
      command_buffer.bindIndexBuffer(scene_.index_buffer().buffer(), 0, vk::IndexType::eUint32);
      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.layout(), 0,
         {
            descriptor_sets_.sets("camera")[current_frame_],
            descriptor_sets_.sets("texturing").front()
         }, {});
      for (auto const& [vertex_offset, index_offset, index_count, material_index] : scene_.sub_meshes())
      {
         command_buffer.pushConstants<std::uint32_t>(
            *pipeline_.layout(),
            vk::ShaderStageFlagBits::eFragment,
            0,
            material_index);

         command_buffer.drawIndexed(
            index_count,
            1,
            index_offset,
            vertex_offset,
            0);
      }

      command_buffer.endRendering();

      vk::ImageMemoryBarrier2 const present_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .srcAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eBottomOfPipe },
         .dstAccessMask{ vk::AccessFlagBits2::eNone },
         .oldLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .newLayout{ vk::ImageLayout::ePresentSrcKHR },
         .image{ image.image() },
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

      command_buffer.end();

      std::array<vk::PipelineStageFlags, 1> constexpr wait_stages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
      device_.queues().front().queue().submit({
         {
            .waitSemaphoreCount{ 1 },
            .pWaitSemaphores{ &*image_available_semaphore },
            .pWaitDstStageMask{ wait_stages.data() },
            .commandBufferCount{ 1 },
            .pCommandBuffers{ &*command_buffers_[current_frame_] },
            .signalSemaphoreCount{ 1 },
            .pSignalSemaphores{ &*render_finished_semaphore },
         }
      }, command_buffer_executed_fence);

      try
      {
         if (device_.queues().front().queue().presentKHR({
            .waitSemaphoreCount{ 1 },
            .pWaitSemaphores{ &*render_finished_semaphore },
            .swapchainCount{ 1 },
            .pSwapchains{ &*swap_chain_.swap_chain() },
            .pImageIndices{ &image_index }
         }) not_eq vk::Result::eSuccess)
            exception("failed to present!");
      }
      catch (vk::OutOfDateKHRError const&)
      {
         device_.device().waitIdle();
         swap_chain_builder_.change_old_swap_chain(&swap_chain_);
         swap_chain_ = swap_chain_builder_.build(device_, *window_, device_.queues());

         depth_pass_.recreate_depth_image(device_, swap_chain_.extent());

         camera.change_projection_extent(swap_chain_.extent());
      }

      current_frame_ = (current_frame_ + 1) % FRAMES_IN_FLIGHT;
   }
}