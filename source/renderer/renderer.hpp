#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "buffer.hpp"
#include "builders/buffer_builder.hpp"
#include "builders/device_builder.hpp"
#include "builders/image_builder.hpp"
#include "builders/image_view_builder.hpp"
#include "builders/pipeline_builder.hpp"
#include "builders/swap_chain_builder.hpp"
#include "camera.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "scene/scene.hpp"
#include "scene/vertex.hpp"
#include "shader.hpp"
#include "window/window.hpp"

namespace eru
{
   class Renderer final
   {
      public:
         explicit Renderer(Window const& window);
         Renderer(Renderer const&) = delete;
         Renderer(Renderer&&) = delete;

         ~Renderer();

         Renderer& operator=(Renderer const&) = delete;
         Renderer& operator=(Renderer&&) = delete;

         void render();

         Camera camera{};

      private:
         std::uint32_t const frames_in_flight_{ 3 };

         Reference<Window const> const window_;

         Device device_{
            DeviceBuilder{}
            .enable_extensions({
               vk::KHRSwapchainExtensionName,
               vk::KHRSynchronization2ExtensionName,
               vk::KHRDynamicRenderingExtensionName
            })
            .enable_features({ .samplerAnisotropy{ true } })
            .add_queues({ vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, window_->surface() })
            .build()
         };

         SwapChainBuilder swap_chain_builder_{};
         SwapChain swap_chain_{
            swap_chain_builder_
            .change_format({ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
            .change_present_mode(vk::PresentModeKHR::eMailbox)
            .build(device_, *window_, device_.queues())
         };

         Shader vertex_shader_{ "resources/shaders/test.vert", device_ };
         Shader fragment_shader_{ "resources/shaders/test.frag", device_ };
         Pipeline pipeline_{
            PipelineBuilder{}
            .change_color_attachment_format(swap_chain_.images().front().info().format)
            .add_vertex_bindings(Vertex::BINDING_DESCRIPTIONS)
            .add_vertex_attributes(Vertex::ATTRIBUTE_DESCRIPTIONS)
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
            .change_depth_attachment_format(vk::Format::eD32Sfloat)
            .build(device_)
         };

         std::vector<vk::raii::CommandBuffer> command_buffers_{
            device_.device().allocateCommandBuffers({
               .commandPool{ *device_.command_pool(device_.queues().front()) },
               .level{ vk::CommandBufferLevel::ePrimary },
               .commandBufferCount{ 3 }
            })
         };

         std::vector<vk::raii::Semaphore> image_available_semaphores_{
            [this]
            {
               std::vector<vk::raii::Semaphore> semaphores{};
               semaphores.reserve(frames_in_flight_);
               for (std::size_t index{}; index < frames_in_flight_; ++index)
                  semaphores.emplace_back(device_.device().createSemaphore({}));

               return semaphores;
            }()
         };

         std::vector<vk::raii::Semaphore> render_finished_semaphores_{
            [this]
            {
               std::vector<vk::raii::Semaphore> semaphores{};
               semaphores.reserve(frames_in_flight_);
               for (std::size_t index{}; index < frames_in_flight_; ++index)
                  semaphores.emplace_back(device_.device().createSemaphore({}));

               return semaphores;
            }()
         };

         std::vector<vk::raii::Fence> command_buffer_executed_fences_{
            [this]
            {
               std::vector<vk::raii::Fence> fences{};
               fences.reserve(frames_in_flight_);
               for (std::size_t index{}; index < frames_in_flight_; ++index)
                  fences.emplace_back(device_.device().createFence({
                     .flags{ vk::FenceCreateFlagBits::eSignaled }
                  }));

               return fences;
            }()
         };

         std::vector<Buffer> camera_buffers_{
            [this]
            {
               BufferBuilder buffer_builder{};

               vk::DeviceSize constexpr buffer_size{ sizeof(Camera::Data) };
               buffer_builder
                  .change_size(buffer_size)
                  .change_usage(vk::BufferUsageFlagBits::eUniformBuffer)
                  .change_sharing_mode(vk::SharingMode::eExclusive)
                  .change_allocation_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT)
                  .change_allocation_required_flags(
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

               std::vector<Buffer> buffers{};
               buffers.reserve(frames_in_flight_);

               for (std::size_t index{}; index < frames_in_flight_; ++index)
               {
                  vk::DescriptorBufferInfo const buffer_info{
                     .buffer{ buffers.emplace_back(buffer_builder.build(device_)).buffer() },
                     .offset{ 0 },
                     .range{ buffer_size }
                  };

                  device_.device().updateDescriptorSets({
                     {
                        .dstSet{ pipeline_.descriptor_sets()[index] },
                        .dstBinding{ 0 },
                        .dstArrayElement{ 0 },
                        .descriptorCount{ 1 },
                        .descriptorType{ vk::DescriptorType::eUniformBuffer },
                        .pBufferInfo{ &buffer_info }
                     }
                  }, {});
               }

               return buffers;
            }()
         };

         ImageBuilder depth_image_builder_{
            ImageBuilder{}
            .change_type(vk::ImageType::e2D)
            .change_format(vk::Format::eD32Sfloat)
            .change_extent(swap_chain_.extent())
            .change_mip_levels(1)
            .change_array_layers(1)
            .change_samples(vk::SampleCountFlagBits::e1)
            .change_tiling(vk::ImageTiling::eOptimal)
            .change_usage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
            .change_sharing_mode(vk::SharingMode::eExclusive)
            .change_initial_layout(vk::ImageLayout::eUndefined)
            .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
         };

         Image depth_image_{ depth_image_builder_.build(device_) };

         ImageViewBuilder depth_image_view_builder_{
            ImageViewBuilder{}
            .change_view_type(vk::ImageViewType::e2D)
            .change_format(depth_image_.info().format)
            .change_subresource_range({
               .aspectMask{ vk::ImageAspectFlagBits::eDepth },
               .levelCount{ 1 },
               .layerCount{ 1 }
            })
         };

         ImageView depth_image_view_{ depth_image_view_builder_.build(device_, depth_image_) };

         Scene scene_{ device_, "resources/models/sponza/sponza.obj" };

         std::uint32_t current_frame_{};
   };
}

#endif