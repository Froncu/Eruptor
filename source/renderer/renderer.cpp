#include "builders/device_builder.hpp"
#include "builders/pipeline_builder.hpp"
#include "renderer.hpp"

namespace eru
{
   Renderer::Renderer(Window const& window)
      : window_{ window }
      , device_{
         DeviceBuilder{}
         .enable_extensions({
            vk::KHRSwapchainExtensionName,
            vk::KHRSynchronization2ExtensionName,
            vk::KHRDynamicRenderingExtensionName
         })
         .enable_features({ .samplerAnisotropy{ true } })
         .add_queues({ vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, window_->surface() })
         .build()
      }
      , swap_chain_{
         swap_chain_builder_
         .change_format({ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
         .change_present_mode(vk::PresentModeKHR::eMailbox)
         .build(device_, *window_, device_.queues())
      }
      , pipeline_{
         PipelineBuilder{}
         .change_color_attachment_format(swap_chain_.images().front().info().format)
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
            }
         })
         .change_rasterization_state({
            .polygonMode{ vk::PolygonMode::eFill },
            .cullMode{ vk::CullModeFlagBits::eBack },
            .frontFace{ vk::FrontFace::eClockwise },
            .lineWidth{ 1.0f }
         })
         .change_descriptor_set_count(frames_in_flight_)
         .build(device_)
      }
   {
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

      vk::RenderingAttachmentInfo color_attachment_info{
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

      command_buffer.beginRendering({
         .renderArea{
            .extent{ swap_chain_.extent() }
         },
         .layerCount{ 1 },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &color_attachment_info }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.pipeline());

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.layout(), 0,
         { pipeline_.descriptor_sets()[image_index] }, {});

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

      command_buffer.draw(3, 1, 0, 0);

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

      Camera const camera{
         .view{ glm::lookAt<float, glm::defaultp>({ 0.0f, -2.0f, 2.0 }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }) },
         .projection{
            glm::perspective(glm::radians(45.0f), swap_chain_.extent().width / static_cast<float>(swap_chain_.extent().height),
               0.1f, 8.0f)
         }
      };

      VmaAllocationInfo allocation_info;
      vmaGetAllocationInfo(camera_buffers_[current_frame_].allocator(), camera_buffers_[current_frame_].allocation(),
         &allocation_info);
      std::memcpy(allocation_info.pMappedData, &camera, sizeof(camera));

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
      }

      current_frame_ = (current_frame_ + 1) % frames_in_flight_;
   }
}