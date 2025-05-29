#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "builders/image_view_builder.hpp"
#include "builders/swap_chain_builder.hpp"
#include "device.hpp"
#include "image_view.hpp"
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
               semaphores.reserve(3);
               for (std::size_t index{}; index < 3; ++index)
                  semaphores.emplace_back(device_.device().createSemaphore({}));

               return semaphores;
            }()
         };

         std::vector<vk::raii::Semaphore> render_finished_semaphores_{
            [this]
            {
               std::vector<vk::raii::Semaphore> semaphores{};
               semaphores.reserve(3);
               for (std::size_t index{}; index < 3; ++index)
                  semaphores.emplace_back(device_.device().createSemaphore({}));

               return semaphores;
            }()
         };

         std::vector<vk::raii::Fence> command_buffer_executed_fences_{
            [this]
            {
               std::vector<vk::raii::Fence> fences{};
               fences.reserve(3);
               for (std::size_t index{}; index < 3; ++index)
                  fences.emplace_back(device_.device().createFence({
                     .flags{ vk::FenceCreateFlagBits::eSignaled }
                  }));

               return fences;
            }()
         };

         std::uint32_t current_frame_{};
   };
}

#endif