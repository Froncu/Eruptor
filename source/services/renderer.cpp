#include "eruptor/context.hpp"
#include "eruptor/exception.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"
#include "eruptor/renderer.hpp"
#include "eruptor/runtime_assert.hpp"

#include "core/dependencies.hpp"

namespace eru
{
   Renderer::Renderer(PassKey<Locator>)
   {
      vk::Result result{ vertex_buffer_.bindMemory(vertex_buffer_memory_, 0) };
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to bind vertex buffer's memory! ({})", to_string(result)));

      vk::DeviceSize const vertex_buffer_size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() };
      vk::raii::Buffer const vertex_staging_buffer{
         buffer({
            .size{ vertex_buffer_size },
            .usage{ vk::BufferUsageFlagBits::eTransferSrc },
            .sharingMode{ vk::SharingMode::eExclusive }
         })
      };

      vk::raii::DeviceMemory const vertex_staging_buffer_memory{
         context_.allocate_memory(vertex_staging_buffer.getMemoryRequirements(),
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      result = vertex_staging_buffer.bindMemory(vertex_staging_buffer_memory, 0);
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to bind vertex staging buffer's memory! ({})", to_string(result)));

      vk::ResultValue mapped_memory{ vertex_staging_buffer_memory.mapMemory(0, vertex_buffer_size) };
      RUNTIME_ASSERT(mapped_memory.has_value(),
         std::format("failed to map vertex staging buffer's memory! ({})", to_string(mapped_memory.result)));

      std::memcpy(*mapped_memory, vertices_.data(), vertex_buffer_size);
      vertex_staging_buffer_memory.unmapMemory();

      //

      result = index_buffer_.bindMemory(index_buffer_memory_, 0);
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to bind index buffer's memory! ({})", to_string(result)));

      vk::DeviceSize const index_buffer_size{ sizeof(decltype(indices_)::value_type) * indices_.size() };
      vk::raii::Buffer const index_staging_buffer{
         buffer({
            .size{ index_buffer_size },
            .usage{ vk::BufferUsageFlagBits::eTransferSrc },
            .sharingMode{ vk::SharingMode::eExclusive }
         })
      };

      vk::raii::DeviceMemory const index_staging_buffer_memory{
         context_.allocate_memory(index_staging_buffer.getMemoryRequirements(),
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      result = index_staging_buffer.bindMemory(index_staging_buffer_memory, 0);
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to bind index staging buffer's memory! ({})", to_string(result)));

      mapped_memory = index_staging_buffer_memory.mapMemory(0, index_buffer_size);
      RUNTIME_ASSERT(mapped_memory.has_value(),
         std::format("failed to map index staging buffer's memory! ({})", to_string(mapped_memory.result)));

      std::memcpy(*mapped_memory, indices_.data(), index_buffer_size);
      index_staging_buffer_memory.unmapMemory();

      //

      result = image_.bindMemory(image_memory_, 0);
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to bind image buffer's memory! ({})", to_string(result)));

      image_view_ = image_view();

      vk::DeviceSize const image_buffer_size{ texture_->dataSize };
      vk::raii::Buffer const image_staging_buffer{
         buffer({
            .size{ image_buffer_size },
            .usage{ vk::BufferUsageFlagBits::eTransferSrc },
            .sharingMode{ vk::SharingMode::eExclusive }
         })
      };

      vk::raii::DeviceMemory const image_staging_buffer_memory{
         context_.allocate_memory(image_staging_buffer.getMemoryRequirements(),
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      result = image_staging_buffer.bindMemory(image_staging_buffer_memory, 0);
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to bind image staging buffer's memory! ({})", to_string(result)));

      mapped_memory = image_staging_buffer_memory.mapMemory(0, image_buffer_size);
      RUNTIME_ASSERT(mapped_memory.has_value(),
         std::format("failed to map image staging buffer's memory! ({})", to_string(mapped_memory.result)));

      std::memcpy(*mapped_memory, texture_->pData, image_buffer_size);
      image_staging_buffer_memory.unmapMemory();

      //

      result = depth_image_.bindMemory(depth_image_memory_, 0);
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to bind depth image's memory! ({})", to_string(result)));

      depth_image_view_ = depth_image_view();

      //

      vk::ResultValue const command_buffers{
         context_.device.allocateCommandBuffers({
            .commandPool{ context_.command_pool },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ 1 }
         })
      };
      RUNTIME_ASSERT(command_buffers.has_value(),
         std::format("failed to allocate a command buffer! ({})", to_string(command_buffers.result)));

      result = command_buffers->front().begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to begin command buffer! ({})", to_string(result)));

      std::array const vertex_buffer_copy_regions{
         std::to_array<vk::BufferCopy2>({
            {
               .size{ vertex_buffer_size }
            }
         })
      };
      command_buffers->front().copyBuffer2({
         .srcBuffer{ vertex_staging_buffer },
         .dstBuffer{ vertex_buffer_ },
         .regionCount{ static_cast<std::uint32_t>(std::ranges::size(vertex_buffer_copy_regions)) },
         .pRegions{ std::ranges::data(vertex_buffer_copy_regions) }
      });

      std::array const index_buffer_copy_regions{
         std::to_array<vk::BufferCopy2>({
            {
               .size{ index_buffer_size }
            }
         })
      };
      command_buffers->front().copyBuffer2({
         .srcBuffer{ index_staging_buffer },
         .dstBuffer{ index_buffer_ },
         .regionCount{ static_cast<std::uint32_t>(std::ranges::size(index_buffer_copy_regions)) },
         .pRegions{ std::ranges::data(index_buffer_copy_regions) }
      });

      std::array image_memory_barriers{
         std::to_array<vk::ImageMemoryBarrier2>({
            {
               .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
               .srcAccessMask{ vk::AccessFlagBits2::eNone },
               .dstStageMask{ vk::PipelineStageFlagBits2::eTransfer },
               .dstAccessMask{ vk::AccessFlagBits2::eTransferWrite },
               .oldLayout{ vk::ImageLayout::eUndefined },
               .newLayout{ vk::ImageLayout::eTransferDstOptimal },
               .image{ image_ },
               .subresourceRange{
                  .aspectMask{ vk::ImageAspectFlagBits::eColor },
                  .baseMipLevel{ 0 },
                  .levelCount{ 1 },
                  .baseArrayLayer{ 0 },
                  .layerCount{ 1 }
               },
            },
            {
               .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
               .srcAccessMask{ vk::AccessFlagBits2::eNone },
               .dstStageMask{ vk::PipelineStageFlagBits2::eNone },
               .dstAccessMask{ vk::AccessFlagBits2::eNone },
               .oldLayout{ vk::ImageLayout::eUndefined },
               .newLayout{ vk::ImageLayout::eDepthAttachmentOptimal },
               .image{ depth_image_ },
               .subresourceRange{
                  .aspectMask{ vk::ImageAspectFlagBits::eDepth },
                  .baseMipLevel{ 0 },
                  .levelCount{ 1 },
                  .baseArrayLayer{ 0 },
                  .layerCount{ 1 }
               },
            }
         })
      };
      command_buffers->front().pipelineBarrier2({
         .imageMemoryBarrierCount{ static_cast<std::uint32_t>(std::ranges::size(image_memory_barriers)) },
         .pImageMemoryBarriers{ std::ranges::data(image_memory_barriers) }
      });

      std::array const image_buffer_copy_regions{
         std::to_array<vk::BufferImageCopy2>({
            {
               .imageSubresource{
                  .aspectMask{ vk::ImageAspectFlagBits::eColor },
                  .mipLevel{ 0 },
                  .baseArrayLayer{ 0 },
                  .layerCount{ 1 }
               },
               .imageExtent{
                  .width{ texture_->baseWidth },
                  .height{ texture_->baseHeight },
                  .depth{ texture_->baseDepth }
               }
            }
         })
      };
      command_buffers->front().copyBufferToImage2({
         .srcBuffer{ image_staging_buffer },
         .dstImage{ image_ },
         .dstImageLayout{ vk::ImageLayout::eTransferDstOptimal },
         .regionCount{ static_cast<std::uint32_t>(std::ranges::size(image_buffer_copy_regions)) },
         .pRegions{ std::ranges::data(image_buffer_copy_regions) }
      });

      std::array end_image_memory_barriers =
         std::to_array<vk::ImageMemoryBarrier2>({
            {
               .srcStageMask{ vk::PipelineStageFlagBits2::eTransfer },
               .srcAccessMask{ vk::AccessFlagBits2::eTransferWrite },
               .dstStageMask{ vk::PipelineStageFlagBits2::eNone },
               .dstAccessMask{ vk::AccessFlagBits2::eNone },
               .oldLayout{ vk::ImageLayout::eTransferDstOptimal },
               .newLayout{ vk::ImageLayout::eShaderReadOnlyOptimal },
               .image{ image_ },
               .subresourceRange{
                  .aspectMask{ vk::ImageAspectFlagBits::eColor },
                  .baseMipLevel{ 0 },
                  .levelCount{ 1 },
                  .baseArrayLayer{ 0 },
                  .layerCount{ 1 }
               }
            }
         });
      command_buffers->front().pipelineBarrier2({
         .imageMemoryBarrierCount{ static_cast<std::uint32_t>(std::ranges::size(end_image_memory_barriers)) },
         .pImageMemoryBarriers{ std::ranges::data(end_image_memory_barriers) }
      });

      result = command_buffers->front().end();
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to end command buffer! ({})", to_string(result)));

      context_.queue.submit({
         {
            {
               .commandBufferCount{ 1 },
               .pCommandBuffers{ &*command_buffers->front() },
            }
         }
      });
      result = context_.queue.waitIdle();
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to wait for queue! ({})", to_string(result)));

      //

      uniform_buffer_mapped_.reserve(FRAMES_IN_FLIGHT);
      std::vector<vk::DescriptorBufferInfo> buffer_infos{};
      buffer_infos.reserve(FRAMES_IN_FLIGHT);
      std::vector<vk::WriteDescriptorSet> writes{};
      writes.reserve(FRAMES_IN_FLIGHT + 2);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
      {
         uniform_buffers_[index].bindMemory(uniform_buffer_memories_[index], 0);

         mapped_memory = uniform_buffer_memories_[index].mapMemory(0, sizeof(UniformBufferObject));
         RUNTIME_ASSERT(mapped_memory.has_value(),
            std::format("failed to map a uniform buffer's memory! ({})", to_string(mapped_memory.result)));

         uniform_buffer_mapped_.push_back(static_cast<UniformBufferObject*>(*mapped_memory));

         buffer_infos.push_back({
            .buffer{ uniform_buffers_[index] },
            .offset{},
            .range{ vk::WholeSize }
         });

         writes.push_back({
            .dstSet{ uniform_buffer_descriptor_sets_[index] },
            .dstBinding{ 0 },
            .dstArrayElement{ 0 },
            .descriptorCount{ 1 },
            .descriptorType{ vk::DescriptorType::eUniformBuffer },
            .pImageInfo{ nullptr },
            .pBufferInfo{ &buffer_infos[index] },
            .pTexelBufferView{ nullptr }
         });
      }

      vk::DescriptorImageInfo const sampler_info{
         .sampler{ sampler_ }
      };

      writes.push_back({
         .dstSet{ sampler_descriptor_set_ },
         .dstBinding{ 0 },
         .dstArrayElement{ 0 },
         .descriptorCount{ 1 },
         .descriptorType{ vk::DescriptorType::eSampler },
         .pImageInfo{ &sampler_info },
         .pBufferInfo{ nullptr },
         .pTexelBufferView{ nullptr }
      });

      vk::DescriptorImageInfo const image_info{
         .imageView{ image_view_ },
         .imageLayout{ vk::ImageLayout::eShaderReadOnlyOptimal },
      };

      writes.push_back({
         .dstSet{ sampler_descriptor_set_ },
         .dstBinding{ 1 },
         .dstArrayElement{ 0 },
         .descriptorCount{ 1 },
         .descriptorType{ vk::DescriptorType::eSampledImage },
         .pImageInfo{ &image_info },
         .pBufferInfo{ nullptr },
         .pTexelBufferView{ nullptr }
      });

      context_.device.updateDescriptorSets(writes, {});
   }

   Renderer::~Renderer()
   {
      vk::Result const result{ context_.device.waitIdle() };
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to wait idle on the device! ({})", to_string(result)));
   }

   auto Renderer::render(vk::Image const image, vk::ImageView const image_view, vk::Extent2D extent) -> void
   {
      // TODO: this will go outside of the renderer
      auto& [model, view, projection]{ *uniform_buffer_mapped_[frame_index_] };

      static auto start_time{ std::chrono::high_resolution_clock::now() };

      auto const current_time{ std::chrono::high_resolution_clock::now() };
      float const time{ std::chrono::duration<float>(current_time - start_time).count() };

      model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      projection = glm::perspective(glm::radians(45.0f), static_cast<float>(extent.width) / extent.height, 0.1f, 10.0f);
      projection[1][1] *= -1;

      //

      vk::raii::CommandBuffer const& command_buffer{ command_buffers_[frame_index_] };
      vk::raii::Semaphore const& image_available_semaphore{ image_available_semaphores_[frame_index_] };
      vk::raii::Semaphore const& command_buffer_finished_semaphore{ command_buffer_finished_semaphores_[frame_index_] };

      vk::Result result = command_buffer.begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to begin command buffer! ({})", to_string(result)));

      vk::ImageMemoryBarrier2 const begin_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .oldLayout{ vk::ImageLayout::eUndefined },
         .newLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .image{ image },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &begin_barrier }
      });

      vk::RenderingAttachmentInfo const attachment_info{
         .imageView{ image_view },
         .imageLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .clearValue{ vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f } }
      };

      vk::RenderingAttachmentInfo const depth_attachment_info{
         .imageView{ depth_image_view_ },
         .imageLayout{ vk::ImageLayout::eDepthAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .clearValue{ vk::ClearColorValue{ 1.0f, 1.0f, 1.0f, 1.0f } }
      };

      command_buffer.beginRendering({
         .renderArea{
            .extent{ extent }
         },
         .layerCount{ 1 },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &attachment_info },
         .pDepthAttachment{ &depth_attachment_info }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);
      command_buffer.bindVertexBuffers(0, { vertex_buffer_ }, { 0 });
      command_buffer.bindIndexBuffer(*index_buffer_, 0, vk::IndexType::eUint16);
      command_buffer.setViewport(0, {
         {
            .width{ static_cast<float>(extent.width) },
            .height{ static_cast<float>(extent.height) },
            .maxDepth{ 1.0f },
         }
      });

      command_buffer.setScissor(0, {
         {
            .extent{ extent }
         }
      });

      std::array const descriptor_sets{
         std::to_array<vk::DescriptorSet>({
            *uniform_buffer_descriptor_sets_[frame_index_],
            *sampler_descriptor_set_
         })
      };
      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0, descriptor_sets, nullptr);
      command_buffer.drawIndexed(static_cast<std::uint32_t>(indices_.size()), 1, 0, 0, 0);
      command_buffer.endRendering();

      vk::ImageMemoryBarrier2 const end_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .srcAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits2::eNone },
         .oldLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .newLayout{ vk::ImageLayout::ePresentSrcKHR },
         .image{ image },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2(
         {
            .imageMemoryBarrierCount{ 1 },
            .pImageMemoryBarriers{ &end_barrier }
         });

      result = command_buffer.end();
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to end command buffer! ({})", to_string(result)));

      std::array const wait_semaphore_infos{
         std::to_array<vk::SemaphoreSubmitInfo>({
            {
               .semaphore{ image_available_semaphore },
               .stageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
            }
         })
      };

      std::array const command_buffer_infos{
         std::to_array<vk::CommandBufferSubmitInfo>({
            {
               .commandBuffer{ command_buffer }
            }
         })
      };

      std::array const signal_semaphore_infos{
         std::to_array<vk::SemaphoreSubmitInfo>({
            {
               .semaphore{ command_buffer_finished_semaphore },
               .stageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
            }
         })
      };

      result = context_.queue.submit2({
         vk::SubmitInfo2{
            .waitSemaphoreInfoCount{ static_cast<std::uint32_t>(std::ranges::size(wait_semaphore_infos)) },
            .pWaitSemaphoreInfos{ std::ranges::data(wait_semaphore_infos) },
            .commandBufferInfoCount{ static_cast<std::uint32_t>(std::ranges::size(command_buffer_infos)) },
            .pCommandBufferInfos{ std::ranges::data(command_buffer_infos) },
            .signalSemaphoreInfoCount{ static_cast<std::uint32_t>(std::ranges::size(signal_semaphore_infos)) },
            .pSignalSemaphoreInfos{ std::ranges::data(signal_semaphore_infos) }
         }
      });
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to submit command buffer! ({})", to_string(result)));

      ++frame_index_ %= FRAMES_IN_FLIGHT;
   }

   auto Renderer::depth_image() const -> vk::raii::Image
   {
      vk::ResultValue image{
         context_.device.createImage({
            .imageType{ vk::ImageType::e2D },
            .format{ vk::Format::eD16Unorm },
            .extent{
               .width{ surface_extent_.width },
               .height{ surface_extent_.height },
               .depth{ 1 }
            },
            .mipLevels{ 1 },
            .arrayLayers{ 1 },
            .samples{ vk::SampleCountFlagBits::e1 },
            .tiling{ vk::ImageTiling::eOptimal },
            .usage{ vk::ImageUsageFlagBits::eDepthStencilAttachment },
            .sharingMode{ vk::SharingMode::eExclusive },
            .initialLayout{ vk::ImageLayout::eUndefined },
         })
      };
      RUNTIME_ASSERT(image.result == vk::Result::eSuccess,
         std::format("failed to create depth image! ({})", to_string(image.result)));

      return std::move(*image);
   }

   auto Renderer::depth_image_view() const -> vk::raii::ImageView
   {
      vk::ResultValue image_view{
         context_.device.createImageView({
            .image{ depth_image_ },
            .viewType{ vk::ImageViewType::e2D },
            .format{ vk::Format::eD16Unorm },
            .subresourceRange{
               .aspectMask{ vk::ImageAspectFlagBits::eDepth },
               .baseMipLevel{ 0 },
               .levelCount{ 1 },
               .baseArrayLayer{ 0 },
               .layerCount{ 1 }
            }
         })
      };
      RUNTIME_ASSERT(image_view.result == vk::Result::eSuccess,
         std::format("failed to create depth image view! ({})", to_string(image_view.result)));

      return std::move(*image_view);
   }

   auto Renderer::depth_image_memory() const -> vk::raii::DeviceMemory
   {
      return context_.allocate_memory(depth_image_.getMemoryRequirements(), vk::MemoryPropertyFlagBits::eDeviceLocal);
   }

   auto Renderer::uniform_buffer_descriptor_set_layout() const -> vk::raii::DescriptorSetLayout
   {
      std::array constexpr bindings{
         std::to_array<vk::DescriptorSetLayoutBinding>({
            {
               .binding{ 0 },
               .descriptorType{ vk::DescriptorType::eUniformBuffer },
               .descriptorCount{ 1 },
               .stageFlags{ vk::ShaderStageFlagBits::eVertex }
            }
         })
      };

      vk::ResultValue descriptor_set_layout{
         context_.device.createDescriptorSetLayout({
            .bindingCount{ static_cast<std::uint32_t>(std::ranges::size(bindings)) },
            .pBindings{ std::ranges::data(bindings) }
         })
      };
      RUNTIME_ASSERT(descriptor_set_layout.has_value(),
         std::format("failed to create uniform buffer descriptor set layout! ({})", to_string(descriptor_set_layout.result)));

      return std::move(*descriptor_set_layout);
   }

   auto Renderer::sampler_descriptor_set_layout() const -> vk::raii::DescriptorSetLayout
   {
      std::array constexpr bindings{
         std::to_array<vk::DescriptorSetLayoutBinding>({
            {
               .binding{ 0 },
               .descriptorType{ vk::DescriptorType::eSampler },
               .descriptorCount{ 1 },
               .stageFlags{ vk::ShaderStageFlagBits::eFragment }
            },
            {
               .binding{ 1 },
               .descriptorType{ vk::DescriptorType::eSampledImage },
               .descriptorCount{ 1 },
               .stageFlags{ vk::ShaderStageFlagBits::eFragment }
            }
         })
      };

      vk::ResultValue descriptor_set_layout{
         context_.device.createDescriptorSetLayout({
            .bindingCount{ static_cast<std::uint32_t>(std::ranges::size(bindings)) },
            .pBindings{ std::ranges::data(bindings) }
         })
      };
      RUNTIME_ASSERT(descriptor_set_layout.has_value(),
         std::format("failed to create sampler descriptor set layout! ({})", to_string(descriptor_set_layout.result)));

      return std::move(*descriptor_set_layout);
   }

   auto Renderer::pipeline_layout() const -> vk::raii::PipelineLayout
   {
      std::array const layouts{
         std::to_array<vk::DescriptorSetLayout>({
            *uniform_buffer_descriptor_set_layout_,
            *sampler_descriptor_set_layout_
         })
      };

      vk::ResultValue pipeline_layout{
         context_.device.createPipelineLayout({
            .setLayoutCount{ static_cast<std::uint32_t>(std::ranges::size(layouts)) },
            .pSetLayouts{ std::ranges::data(layouts) }
         })
      };
      RUNTIME_ASSERT(pipeline_layout.has_value(),
         std::format("failed to create a pipeline layout! ({})", to_string(pipeline_layout.result)));

      return std::move(pipeline_layout.value);
   }

   auto Renderer::pipeline() const -> vk::raii::Pipeline
   {
      Slang::ComPtr<slang::IGlobalSession> global_session;
      createGlobalSession(global_session.writeRef());
      RUNTIME_ASSERT(global_session,
         std::format("failed to create a global session! ({})", slang::getLastInternalErrorMessage()));

      std::array slang_targets{
         std::to_array<slang::TargetDesc>({
            {
               .format{ SLANG_SPIRV }
            }
         })
      };

      slang::SessionDesc const slang_session_description{
         .targets{ std::ranges::data(slang_targets) },
         .targetCount{ static_cast<SlangInt>(std::ranges::size(slang_targets)) },
         .defaultMatrixLayoutMode{ SLANG_MATRIX_LAYOUT_COLUMN_MAJOR }
      };

      Slang::ComPtr<slang::ISession> slang_session;
      global_session->createSession(slang_session_description, slang_session.writeRef());
      RUNTIME_ASSERT(slang_session,
         std::format("failed to create a slang session! ({})", slang::getLastInternalErrorMessage()));

      Slang::ComPtr const slang_module{
         slang_session->loadModuleFromSource("shader", "assets/shaders/shader.slang", nullptr, nullptr)
      };
      RUNTIME_ASSERT(slang_module,
         std::format("failed to load a slang module! ({})", slang::getLastInternalErrorMessage()));

      Slang::ComPtr<ISlangBlob> spirv;
      slang_module->getTargetCode(0, spirv.writeRef());
      RUNTIME_ASSERT(spirv,
         std::format("failed to load get target code! ({})", slang::getLastInternalErrorMessage()));

      vk::ShaderModuleCreateInfo const shader_module_create_info{
         .codeSize{ spirv->getBufferSize() },
         .pCode{ static_cast<std::uint32_t const*>(spirv->getBufferPointer()) }
      };

      std::array const shader_stage_create_infos{
         std::to_array<vk::PipelineShaderStageCreateInfo>({
            {
               .pNext{ &shader_module_create_info },
               .stage{ vk::ShaderStageFlagBits::eVertex },
               .pName{ "vertMain" }
            },
            {
               .pNext{ &shader_module_create_info },
               .stage{ vk::ShaderStageFlagBits::eFragment },
               .pName{ "fragMain" }
            }
         })
      };

      std::array constexpr dynamic_states{
         vk::DynamicState::eViewport,
         vk::DynamicState::eScissor
      };

      vk::PipelineDynamicStateCreateInfo const dynamic_state_create_info{
         .dynamicStateCount{ static_cast<uint32_t>(std::ranges::size(dynamic_states)) },
         .pDynamicStates{ std::ranges::data(dynamic_states) }
      };

      vk::PipelineVertexInputStateCreateInfo constexpr vertex_input_state_create_info{
         .vertexBindingDescriptionCount{ static_cast<std::uint32_t>(std::ranges::size(Vertex::INPUT_BINDING_DESCRIPTIONS)) },
         .pVertexBindingDescriptions{ std::ranges::data(Vertex::INPUT_BINDING_DESCRIPTIONS) },
         .vertexAttributeDescriptionCount{
            static_cast<std::uint32_t>(std::ranges::size(Vertex::INPUT_ATTRIBUTE_DESCRIPTIONS))
         },
         .pVertexAttributeDescriptions{ std::ranges::data(Vertex::INPUT_ATTRIBUTE_DESCRIPTIONS) }
      };

      vk::PipelineInputAssemblyStateCreateInfo constexpr input_assembly_state_create_info{
         .topology{ vk::PrimitiveTopology::eTriangleStrip }
      };

      vk::PipelineViewportStateCreateInfo constexpr viewport_state_create_info{
         .viewportCount{ 1 },
         .scissorCount{ 1 }
      };

      vk::PipelineRasterizationStateCreateInfo constexpr rasterization_state_create_info{
         .depthClampEnable{ vk::False },
         .rasterizerDiscardEnable{ vk::False },
         .polygonMode{ vk::PolygonMode::eFill },
         .cullMode{ vk::CullModeFlagBits::eBack },
         .frontFace{ vk::FrontFace::eCounterClockwise },
         .depthBiasEnable{ vk::False },
         .depthBiasSlopeFactor{ 1.0f },
         .lineWidth{ 1.0f }
      };

      vk::PipelineMultisampleStateCreateInfo constexpr multisample_state_create_info{
         .rasterizationSamples{ vk::SampleCountFlagBits::e1 },
         .sampleShadingEnable{ vk::False }
      };

      vk::PipelineDepthStencilStateCreateInfo constexpr depth_stencil_state_create_info{
         .depthTestEnable{ vk::True },
         .depthWriteEnable{ vk::True },
         .depthCompareOp{ vk::CompareOp::eLess },
         .depthBoundsTestEnable{ vk::False },
         .stencilTestEnable{ vk::False },
      };

      std::array constexpr color_blend_attachment_state{
         std::to_array<vk::PipelineColorBlendAttachmentState>({
            {
               .blendEnable{ vk::False },
               .colorWriteMask{
                  vk::ColorComponentFlagBits::eR |
                  vk::ColorComponentFlagBits::eG |
                  vk::ColorComponentFlagBits::eB |
                  vk::ColorComponentFlagBits::eA
               }
            }
         })
      };

      vk::PipelineColorBlendStateCreateInfo const color_blend_state_create_info{
         .logicOpEnable{ vk::False },
         .logicOp{ vk::LogicOp::eCopy },
         .attachmentCount{ static_cast<std::uint32_t>(std::ranges::size(color_blend_attachment_state)) },
         .pAttachments{ std::ranges::data(color_blend_attachment_state) }
      };

      std::array const color_attachments{
         surface_format_.format,
      };

      vk::PipelineRenderingCreateInfo const pipeline_rendering_create_info{
         .colorAttachmentCount{ static_cast<std::uint32_t>(std::ranges::size(color_attachments)) },
         .pColorAttachmentFormats{ std::ranges::data(color_attachments) },
         .depthAttachmentFormat{ vk::Format::eD16Unorm }
      };

      vk::ResultValue pipeline{
         context_.device.createGraphicsPipeline(nullptr, {
            .pNext{ &pipeline_rendering_create_info },
            .stageCount{ static_cast<std::uint32_t>(std::ranges::size(shader_stage_create_infos)) },
            .pStages{ std::ranges::data(shader_stage_create_infos) },
            .pVertexInputState{ &vertex_input_state_create_info },
            .pInputAssemblyState{ &input_assembly_state_create_info },
            .pViewportState{ &viewport_state_create_info },
            .pRasterizationState{ &rasterization_state_create_info },
            .pMultisampleState{ &multisample_state_create_info },
            .pDepthStencilState{ &depth_stencil_state_create_info },
            .pColorBlendState{ &color_blend_state_create_info },
            .pDynamicState{ &dynamic_state_create_info },
            .layout{ pipeline_layout_ },
         })
      };
      RUNTIME_ASSERT(pipeline.has_value(),
         std::format("failed create a pipeline! ({})", to_string(pipeline.result)));

      return std::move(*pipeline);
   }

   auto Renderer::command_buffers() const -> std::vector<vk::raii::CommandBuffer>
   {
      vk::ResultValue command_buffers{
         context_.device.allocateCommandBuffers({
            .commandPool{ context_.command_pool },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ FRAMES_IN_FLIGHT }
         })
      };
      RUNTIME_ASSERT(command_buffers.has_value(),
         std::format("failed allocate command buffers! ({})", to_string(command_buffers.result)));

      return std::move(*command_buffers);
   }

   auto Renderer::semaphores() const -> std::vector<vk::raii::Semaphore>
   {
      std::vector<vk::raii::Semaphore> semaphores{};
      semaphores.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
      {
         vk::ResultValue semaphore{ context_.device.createSemaphore({}) };
         RUNTIME_ASSERT(semaphore.has_value(),
            std::format("failed create a semaphore! ({})", to_string(semaphore.result)));

         semaphores.push_back(std::move(*semaphore));
      }

      return semaphores;
   }

   auto Renderer::fences() const -> std::vector<vk::raii::Fence>
   {
      std::vector<vk::raii::Fence> fences{};
      fences.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
      {
         vk::ResultValue fence{
            context_.device.createFence({
               .flags{ vk::FenceCreateFlagBits::eSignaled }
            })
         };
         RUNTIME_ASSERT(fence.has_value(),
            std::format("failed create a fence! ({})", to_string(fence.result)));

         fences.push_back(std::move(*fence));
      }

      return fences;
   }

   auto Renderer::buffer(vk::BufferCreateInfo const& create_info) const -> vk::raii::Buffer
   {
      vk::ResultValue buffer{ context_.device.createBuffer(create_info) };
      RUNTIME_ASSERT(buffer.has_value(),
         std::format("failed to create vertex buffer! ({})", to_string(buffer.result)));

      return std::move(*buffer);
   }

   auto Renderer::vertex_buffer() const -> vk::raii::Buffer
   {
      return buffer({
         .size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() },
         .usage{ vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst },
         .sharingMode{ vk::SharingMode::eExclusive }
      });
   }

   auto Renderer::vertex_buffer_memory() const -> vk::raii::DeviceMemory
   {
      return context_.allocate_memory(vertex_buffer_.getMemoryRequirements(), vk::MemoryPropertyFlagBits::eDeviceLocal);
   }

   auto Renderer::index_buffer() const -> vk::raii::Buffer
   {
      return buffer({
         .size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() },
         .usage{ vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst },
         .sharingMode{ vk::SharingMode::eExclusive }
      });
   }

   auto Renderer::index_buffer_memory() const -> vk::raii::DeviceMemory
   {
      return context_.allocate_memory(index_buffer_.getMemoryRequirements(), vk::MemoryPropertyFlagBits::eDeviceLocal);
   }

   auto Renderer::uniform_buffers() const -> std::vector<vk::raii::Buffer>
   {
      std::vector<vk::raii::Buffer> uniform_buffers{};
      uniform_buffers.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
         uniform_buffers.push_back(
            buffer({
               .size{ sizeof(UniformBufferObject) },
               .usage{ vk::BufferUsageFlagBits::eUniformBuffer }
            }));

      return uniform_buffers;
   }

   auto Renderer::uniform_buffer_memories() const -> std::vector<vk::raii::DeviceMemory>
   {
      std::vector<vk::raii::DeviceMemory> uniform_buffer_memories{};
      uniform_buffer_memories.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
         uniform_buffer_memories.push_back(
            context_.allocate_memory(uniform_buffers_[index].getMemoryRequirements(),
               vk::MemoryPropertyFlagBits::eDeviceLocal |
               vk::MemoryPropertyFlagBits::eHostVisible |
               vk::MemoryPropertyFlagBits::eHostCoherent));

      return uniform_buffer_memories;
   }

   auto Renderer::texture(std::string_view const path) const -> UniquePointer<ktxTexture2>
   {
      if (not std::filesystem::exists(path))
         throw Exception{ std::format("\"{}\" does not exist!", path) };

      if (not std::filesystem::is_regular_file(path))
         throw Exception{ std::format("\"{}\" is not a regular file!", path) };

      int width{};
      int height{};
      constexpr auto desired_channels{ STBI_rgb_alpha };
      UniquePointer<stbi_uc> const raw_image{ stbi_load(path.data(), &width, &height, nullptr, desired_channels), stbi_image_free };
      if (not raw_image)
         throw Exception{ std::format("failed to load image! ({})", stbi_failure_reason()) };

      UniquePointer<ktxTexture2> texture{
         [width, height] -> ktxTexture2*
         {
            ktxTextureCreateInfo const create_info{
               .glInternalformat{},
               .vkFormat{ VK_FORMAT_R8G8B8A8_SRGB },
               .pDfd{},
               .baseWidth{ static_cast<ktx_uint32_t>(width) },
               .baseHeight{ static_cast<ktx_uint32_t>(height) },
               .baseDepth{ 1 },
               .numDimensions{ 2 },
               .numLevels{ 1 },
               .numLayers{ 1 },
               .numFaces{ 1 },
               .isArray{ KTX_FALSE },
               .generateMipmaps{ KTX_FALSE }
            };
            ktxTexture2* texture;

            ktx_error_code_e const result{ ktxTexture2_Create(&create_info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture) };
            RUNTIME_ASSERT(result == KTX_SUCCESS, std::format("failed to create ktx texture! ({})", ktxErrorString(result)));

            return texture;
         }(),
         ktxTexture2_Destroy
      };

      ktx_error_code_e const result{
         ktxTexture_SetImageFromMemory(ktxTexture(texture.get()), 0, 0, 0, raw_image.get(), width * height * desired_channels)
      };
      RUNTIME_ASSERT(result == KTX_SUCCESS, std::format("failed to set ktx image data! ({})", ktxErrorString(result)));

      return texture;
   }

   auto Renderer::image() const -> vk::raii::Image
   {
      vk::ResultValue image{
         context_.device.createImage({
            .flags{},
            .imageType{ static_cast<vk::ImageType>(texture_->numDimensions - 1) },
            .format{ static_cast<vk::Format>(texture_->vkFormat) },
            .extent{
               .width{ texture_->baseWidth },
               .height{ texture_->baseHeight },
               .depth{ texture_->baseDepth }
            },
            .mipLevels{ texture_->numLevels },
            .arrayLayers{ texture_->numLayers },
            .samples{ vk::SampleCountFlagBits::e1 },
            .tiling{ vk::ImageTiling::eOptimal },
            .usage{ vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst },
            .sharingMode{ vk::SharingMode::eExclusive },
            .initialLayout{ vk::ImageLayout::eUndefined },
         })
      };
      RUNTIME_ASSERT(image.result == vk::Result::eSuccess, std::format("failed to create image! ({})", to_string(image.result)));

      return std::move(*image);
   }

   auto Renderer::image_view() const -> vk::raii::ImageView
   {
      vk::ResultValue image_view{
         context_.device.createImageView({
            .image{ image_ },
            .viewType{ vk::ImageViewType::e2D },
            .format{ static_cast<vk::Format>(texture_->vkFormat) },
            .subresourceRange{
               .aspectMask{ vk::ImageAspectFlagBits::eColor },
               .baseMipLevel{ 0 },
               .levelCount{ 1 },
               .baseArrayLayer{ 0 },
               .layerCount{ 1 }
            }
         })
      };
      RUNTIME_ASSERT(image_view.result == vk::Result::eSuccess, std::format("failed to create image view! ({})", to_string(image_view.result)));

      return std::move(*image_view);
   }

   auto Renderer::sampler() const -> vk::raii::Sampler
   {
      vk::PhysicalDeviceProperties2 const properties{ context_.physical_device.getProperties2() };
      vk::ResultValue sampler{
         context_.device.createSampler({
            .magFilter{ vk::Filter::eLinear },
            .minFilter{ vk::Filter::eLinear },
            .mipmapMode{ vk::SamplerMipmapMode::eLinear },
            .addressModeU{ vk::SamplerAddressMode::eRepeat },
            .addressModeV{ vk::SamplerAddressMode::eRepeat },
            .addressModeW{ vk::SamplerAddressMode::eRepeat },
            .mipLodBias{},
            .anisotropyEnable{ vk::True },
            .maxAnisotropy{ properties.properties.limits.maxSamplerAnisotropy },
            .compareEnable{ vk::False },
            .compareOp{ vk::CompareOp::eAlways },
            .minLod{ 0.0f },
            .maxLod{ 0.0f },
            .borderColor{ vk::BorderColor::eFloatTransparentBlack },
            .unnormalizedCoordinates{ vk::False }
         })
      };
      RUNTIME_ASSERT(sampler.result == vk::Result::eSuccess, std::format("failed to create sampler! ({})", to_string(sampler.result)));

      return std::move(*sampler);
   }

   auto Renderer::image_memory() const -> vk::raii::DeviceMemory
   {
      return context_.allocate_memory(image_.getMemoryRequirements(), vk::MemoryPropertyFlagBits::eDeviceLocal);
   }

   auto Renderer::descriptor_pool() const -> vk::raii::DescriptorPool
   {
      std::array constexpr desciptor_pool_sizes{
         std::to_array<vk::DescriptorPoolSize>({
            {
               .type{ vk::DescriptorType::eUniformBuffer },
               .descriptorCount{ FRAMES_IN_FLIGHT }
            },
            {
               .type{ vk::DescriptorType::eSampler },
               .descriptorCount{ 1 }
            },
            {
               .type{ vk::DescriptorType::eSampledImage },
               .descriptorCount{ 1 }
            }
         })
      };

      vk::ResultValue descriptor_pool{
         context_.device.createDescriptorPool({
            .flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet },
            .maxSets{ FRAMES_IN_FLIGHT + 1 },
            .poolSizeCount{ static_cast<std::uint32_t>(std::ranges::size(desciptor_pool_sizes)) },
            .pPoolSizes{ std::ranges::data(desciptor_pool_sizes) }
         })
      };
      RUNTIME_ASSERT(descriptor_pool.has_value(),
         std::format("failed to create a descriptor pool! ({})", to_string(descriptor_pool.result)));

      return std::move(*descriptor_pool);
   }

   auto Renderer::uniform_buffer_descriptor_sets() const -> std::vector<vk::raii::DescriptorSet>
   {
      std::array<vk::DescriptorSetLayout, FRAMES_IN_FLIGHT> layouts{};
      std::ranges::fill(layouts, *uniform_buffer_descriptor_set_layout_);

      vk::ResultValue descriptor_sets{
         context_.device.allocateDescriptorSets({
            .descriptorPool{ descriptor_pool_ },
            .descriptorSetCount{ static_cast<std::uint32_t>(std::ranges::size(layouts)) },
            .pSetLayouts{ std::ranges::data(layouts) }
         })
      };
      RUNTIME_ASSERT(descriptor_sets.has_value(),
         std::format("failed to allocate descriptor sets! ({})", to_string(descriptor_sets.result)));

      return std::move(*descriptor_sets);
   }

   auto Renderer::sampler_descriptor_set() const -> vk::raii::DescriptorSet
   {
      vk::ResultValue descriptor_sets{
         context_.device.allocateDescriptorSets({
            .descriptorPool{ descriptor_pool_ },
            .descriptorSetCount{ 1 },
            .pSetLayouts{ &*sampler_descriptor_set_layout_ }
         })
      };
      RUNTIME_ASSERT(descriptor_sets.has_value(),
         std::format("failed to allocate descriptor sets! ({})", to_string(descriptor_sets.result)));

      return std::move(descriptor_sets->front());
   }
}