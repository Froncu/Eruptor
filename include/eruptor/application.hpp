#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/vertex.hpp"
#include "eruptor/window.hpp"

namespace eru
{
   class Application
   {
      class LocatorRegistrator final
      {
         public:
            ERU_API explicit LocatorRegistrator(Application& application);
            LocatorRegistrator(LocatorRegistrator const&) = delete;
            LocatorRegistrator(LocatorRegistrator&&) = delete;

            ERU_API ~LocatorRegistrator();

            LocatorRegistrator& operator=(LocatorRegistrator&&) = delete;
            LocatorRegistrator& operator=(LocatorRegistrator&) = delete;
      };

      class GLFWcontext final
      {
         public:
            ERU_API GLFWcontext();
            GLFWcontext(GLFWcontext const&) = delete;
            GLFWcontext(GLFWcontext&&) = delete;

            ERU_API ~GLFWcontext();

            GLFWcontext& operator=(GLFWcontext&&) = delete;
            GLFWcontext& operator=(GLFWcontext&) = delete;
      };

      static auto constexpr FRAMES_IN_FLIGHT{ 2 };

      public:
         Application(Application const&) = delete;
         Application(Application&&) noexcept = delete;

         ERU_API virtual ~Application();

         Application& operator=(Application const&) = delete;
         Application& operator=(Application&&) = delete;

         ERU_API [[nodiscard]] bool tick();
         ERU_API void poll();

      protected:
         ERU_API explicit Application(std::string_view name = "Eruptor", std::uint32_t version = VK_MAKE_VERSION(0, 0, 0));

      private:
         [[nodiscard]] vk::raii::Instance instance(std::string_view name, std::uint32_t version) const;
         [[nodiscard]] vk::raii::DebugUtilsMessengerEXT debug_messenger() const;
         [[nodiscard]] vk::raii::SurfaceKHR surface() const;
         [[nodiscard]] vk::raii::PhysicalDevice physical_device() const;
         [[nodiscard]] std::uint32_t queue_family_index() const;
         [[nodiscard]] vk::raii::Device device() const;
         [[nodiscard]] vk::raii::Queue queue() const;
         [[nodiscard]] vk::SurfaceFormatKHR surface_format() const;
         [[nodiscard]] vk::Extent2D surface_extent() const;
         [[nodiscard]] vk::raii::SwapchainKHR swap_chain() const;
         [[nodiscard]] std::vector<vk::Image> swap_chain_images() const;
         [[nodiscard]] std::vector<vk::raii::ImageView> swap_chain_image_views() const;
         [[nodiscard]] vk::raii::PipelineLayout pipeline_layout() const;
         [[nodiscard]] vk::raii::Pipeline pipeline() const;
         [[nodiscard]] vk::raii::CommandPool command_pool() const;
         [[nodiscard]] vk::raii::CommandBuffers command_buffers() const;
         [[nodiscard]] std::vector<vk::raii::Semaphore> semaphores() const;
         [[nodiscard]] std::vector<vk::raii::Fence> fences() const;
         [[nodiscard]] vk::raii::Buffer buffer(vk::BufferCreateInfo const& create_info) const;
         [[nodiscard]] vk::raii::DeviceMemory memory(vk::MemoryRequirements const& requirements,
            vk::MemoryPropertyFlags properties) const;
         [[nodiscard]] vk::raii::Buffer vertex_buffer() const;
         [[nodiscard]] vk::raii::DeviceMemory vertex_buffer_memory() const;

         void recreate_swap_chain();

         std::uint8_t frame_index_{};
         std::vector<Vertex> vertices_{
            { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
            { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
            { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
         };

         LocatorRegistrator const locator_registrator_{ *this };
         GLFWcontext const glfw_context_{};
         vk::raii::Context const vulkan_context_{};
         Window window_{ { 1280, 720 }, "Magma" };

         vk::raii::Instance const instance_;
         vk::raii::DebugUtilsMessengerEXT const debug_messenger_{ debug_messenger() };
         vk::raii::SurfaceKHR const surface_{ surface() };
         vk::raii::PhysicalDevice const physical_device_{ physical_device() };
         std::uint32_t const queue_family_index_{ queue_family_index() };
         vk::raii::Device const device_{ device() };
         vk::raii::Queue const queue_{ queue() };
         vk::SurfaceFormatKHR const surface_format_{ surface_format() };
         vk::Extent2D surface_extent_{ surface_extent() };
         vk::raii::SwapchainKHR swap_chain_{ swap_chain() };
         std::vector<vk::Image> swap_chain_images_{ swap_chain_images() };
         std::vector<vk::raii::ImageView> swap_chain_image_views_{ swap_chain_image_views() };
         vk::raii::PipelineLayout const pipeline_layout_{ pipeline_layout() };
         vk::raii::Pipeline const pipeline_{ pipeline() };
         vk::raii::CommandPool const command_pool_{ command_pool() };
         vk::raii::CommandBuffers const command_buffers_{ command_buffers() };
         std::vector<vk::raii::Semaphore> const image_available_semaphores_{ semaphores() };
         std::vector<vk::raii::Semaphore> const command_buffer_finished_semaphores_{ semaphores() };
         std::vector<vk::raii::Fence> const presentation_finished_fences_{ fences() };
         vk::raii::Buffer const vertex_buffer_{ vertex_buffer() };
         vk::raii::DeviceMemory const vertex_buffer_memory_{ vertex_buffer_memory() };
   };
}

#endif