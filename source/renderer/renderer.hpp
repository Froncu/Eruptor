#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "buffer.hpp"
#include "builders/buffer_builder.hpp"
#include "builders/swap_chain_builder.hpp"
#include "camera.hpp"
#include "device.hpp"
#include "pipeline.hpp"
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

      private:
         std::uint32_t const frames_in_flight_{ 3 };

         Reference<Window const> const window_;
         Device device_;
         SwapChainBuilder swap_chain_builder_{};
         SwapChain swap_chain_;
         Shader vertex_shader_{ "resources/shaders/triangle.vert", device_ };
         Shader fragment_shader_{ "resources/shaders/triangle.frag", device_ };
         Pipeline pipeline_;

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

               vk::DeviceSize constexpr buffer_size{ sizeof(Camera) };
               buffer_builder.change_buffer_create_info({
                  .size{ buffer_size },
                  .usage{ vk::BufferUsageFlagBits::eUniformBuffer },
                  .sharingMode{ vk::SharingMode::eExclusive }
               });

               buffer_builder.change_allocation_create_info({
                  .flags{ VMA_ALLOCATION_CREATE_MAPPED_BIT },
                  .usage{},
                  .requiredFlags{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT },
                  .preferredFlags{},
                  .memoryTypeBits{},
                  .pool{},
                  .pUserData{},
                  .priority{}
               });

               std::vector<Buffer> buffers{};
               buffers.reserve(frames_in_flight_);

               for (std::size_t index{}; index < frames_in_flight_; ++index)
               {
                  vk::DescriptorBufferInfo const buffer_info{
                     .buffer{ buffers.emplace_back(buffer_builder.build(device_)).buffer() },
                     .offset{ 0 },
                     .range{ sizeof(Camera) }
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

         std::uint32_t current_frame_{};
   };
}

#endif