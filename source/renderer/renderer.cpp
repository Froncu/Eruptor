#include "renderer.hpp"
#include "scene/material.hpp"

namespace eru
{
   Renderer::Renderer(Window const& window)
      : window_{ window }
   {
      vk::DescriptorBufferInfo const materials_info{
         .buffer{ scene_.materials().buffer() },
         .offset{ 0 },
         .range{ sizeof(Material) * scene_.materials_count() }
      };

      device_.device().updateDescriptorSets({
         {
            .dstSet{ descriptor_sets_.sets("texturing").front() },
            .dstBinding{ descriptor_sets_.binding("texturing", "materials") },
            .dstArrayElement{ 0 },
            .descriptorCount{ 1 },
            .descriptorType{ vk::DescriptorType::eStorageBuffer },
            .pBufferInfo{ &materials_info }
         }
      }, {});

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

      vk::raii::CommandBuffer const& command_buffer{ command_buffers_[current_frame_] };
      vk::raii::Fence const& command_buffer_executed_fence{ command_buffer_executed_fences_[current_frame_] };
      vk::raii::Semaphore const& image_available_semaphore{ image_available_semaphores_[current_frame_] };
      vk::raii::Semaphore const& render_finished_semaphore{ render_finished_semaphores_[current_frame_] };

      if (device_.device().waitForFences({ command_buffer_executed_fence }, true,
         std::numeric_limits<std::uint64_t>::max()) not_eq vk::Result::eSuccess)
         exception("failed to wait for fences!");
      device_.device().resetFences({ command_buffer_executed_fence });

      camera_buffers_[current_frame_].upload(&camera.data(), sizeof(camera.data()));

      auto&& [result, image_index]{
         device_.device().acquireNextImage2KHR({
            .swapchain{ *swap_chain_.swap_chain() },
            .timeout{ std::numeric_limits<std::uint64_t>::max() },
            .semaphore{ *image_available_semaphore },
            .deviceMask{ 1 }
         })
      };

      Image const& swap_chain_image{ swap_chain_.images()[image_index] };
      ImageView const& swap_chain_image_view{ swap_chain_.image_views()[image_index] };

      command_buffer.reset();

      command_buffer.begin({});

      depth_pass_.render(command_buffer, scene_, current_frame_);
      geometry_pass_.render(command_buffer, scene_, depth_pass_.depth_image_views()[current_frame_], current_frame_);
      lighting_pass_.render(command_buffer, camera.position(), current_frame_);
      luminance_pass_.compute(command_buffer, current_frame_);
      tone_map_pass_.render(command_buffer, swap_chain_image, swap_chain_image_view, swap_chain_.extent(), current_frame_);

      command_buffer.end();

      // TODO: this waits on the image to be available in the first color attachment output stage
      // for the current command buffer, which happens in the geometry pass; not good!

      std::array<vk::SemaphoreSubmitInfo, 2> const wait_semaphore_infos{
         {
            {
               .semaphore{ *image_available_semaphore },
               .stageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput }
            },
            {
               .semaphore{ *timeline_semaphore_ },
               .value{ timeline_semaphore_value_++ },
               .stageMask{ vk::PipelineStageFlagBits2::eComputeShader }
            }
         }
      };

      vk::CommandBufferSubmitInfo const command_buffer_info{
         .commandBuffer{ *command_buffers_[current_frame_] }
      };

      std::array<vk::SemaphoreSubmitInfo, 2> const signal_semaphore_infos{
         {
            {
               .semaphore{ *render_finished_semaphore },
               .stageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput }
            },
            {
               .semaphore{ *timeline_semaphore_ },
               .value{ timeline_semaphore_value_ },
               .stageMask{ vk::PipelineStageFlagBits2::eComputeShader },
            }
         }
      };

      device_.queues().front().queue().submit2({
         {
            .waitSemaphoreInfoCount{ static_cast<std::uint32_t>(wait_semaphore_infos.size()) },
            .pWaitSemaphoreInfos{ wait_semaphore_infos.data() },
            .commandBufferInfoCount{ 1 },
            .pCommandBufferInfos{ &command_buffer_info },
            .signalSemaphoreInfoCount{ static_cast<std::uint32_t>(signal_semaphore_infos.size()) },
            .pSignalSemaphoreInfos{ signal_semaphore_infos.data() }
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

         depth_pass_.recreate_depth_images(device_, swap_chain_.extent());
         geometry_pass_.recreate_geometry_images(device_, swap_chain_.extent());
         lighting_pass_.recreate_hdr_images(device_, swap_chain_.extent());

         camera.change_projection_extent(swap_chain_.extent());
      }
      current_frame_ = (current_frame_ + 1) % FRAMES_IN_FLIGHT;
   }
}