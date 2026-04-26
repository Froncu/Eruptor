#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/vertex.hpp"
#include "eruptor/window.hpp"

namespace eru
{
   struct UniformBufferObject
   {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 projection;
   };

   class Application
   {
      class LocatorRegistrator final
      {
         public:
            ERU_API explicit LocatorRegistrator(Application& application);
            LocatorRegistrator(LocatorRegistrator const&) = delete;
            LocatorRegistrator(LocatorRegistrator&&) = delete;

            ERU_API ~LocatorRegistrator();

            auto operator=(LocatorRegistrator&&) -> LocatorRegistrator& = delete;
            auto operator=(LocatorRegistrator&) -> LocatorRegistrator& = delete;
      };

      class GLFWcontext final
      {
         public:
            ERU_API GLFWcontext();
            GLFWcontext(GLFWcontext const&) = delete;
            GLFWcontext(GLFWcontext&&) = delete;

            ERU_API ~GLFWcontext();

            auto operator=(GLFWcontext&&) -> GLFWcontext& = delete;
            auto operator=(GLFWcontext&) -> GLFWcontext& = delete;
      };

      static constexpr auto FRAMES_IN_FLIGHT{ 2 };

      public:
         Application(Application const&) = delete;
         Application(Application&&) noexcept = delete;

         ERU_API virtual ~Application();

         auto operator=(Application const&) -> Application& = delete;
         auto operator=(Application&&) -> Application& = delete;

         [[nodiscard]] ERU_API auto tick() -> bool;
         ERU_API auto poll() -> void;

         bool keep_ticking{ true };

      protected:
         ERU_API explicit Application(std::string_view name = "Eruptor", std::uint32_t version = VK_MAKE_VERSION(0, 0, 0));

      private:
         [[nodiscard]] auto instance(std::string_view name, std::uint32_t version) const -> vk::raii::Instance;
         [[nodiscard]] auto debug_messenger() const -> vk::raii::DebugUtilsMessengerEXT;
         [[nodiscard]] auto surface() const -> vk::raii::SurfaceKHR;
         [[nodiscard]] auto physical_device() const -> vk::raii::PhysicalDevice;
         [[nodiscard]] auto queue_family_index() const -> std::uint32_t;
         [[nodiscard]] auto device() const -> vk::raii::Device;
         [[nodiscard]] auto queue() const -> vk::raii::Queue;
         [[nodiscard]] auto surface_format() const -> vk::SurfaceFormatKHR;
         [[nodiscard]] auto surface_extent() const -> vk::Extent2D;
         [[nodiscard]] auto swap_chain() const -> vk::raii::SwapchainKHR;
         [[nodiscard]] auto swap_chain_images() const -> std::vector<vk::Image>;
         [[nodiscard]] auto swap_chain_image_views() const -> std::vector<vk::raii::ImageView>;
         [[nodiscard]] auto descriptor_set_layout() const -> vk::raii::DescriptorSetLayout;
         [[nodiscard]] auto pipeline_layout() const -> vk::raii::PipelineLayout;
         [[nodiscard]] auto pipeline() const -> vk::raii::Pipeline;
         [[nodiscard]] auto command_pool() const -> vk::raii::CommandPool;
         [[nodiscard]] auto command_buffers() const -> std::vector<vk::raii::CommandBuffer>;
         [[nodiscard]] auto semaphores() const -> std::vector<vk::raii::Semaphore>;
         [[nodiscard]] auto fences() const -> std::vector<vk::raii::Fence>;
         [[nodiscard]] auto buffer(vk::BufferCreateInfo const& create_info) const -> vk::raii::Buffer;
         [[nodiscard]] auto memory(vk::MemoryRequirements const& requirements, vk::MemoryPropertyFlags properties) const -> vk::raii::DeviceMemory;
         [[nodiscard]] auto vertex_buffer() const -> vk::raii::Buffer;
         [[nodiscard]] auto vertex_buffer_memory() const -> vk::raii::DeviceMemory;
         [[nodiscard]] auto index_buffer() const -> vk::raii::Buffer;
         [[nodiscard]] auto index_buffer_memory() const -> vk::raii::DeviceMemory;
         [[nodiscard]] auto uniform_buffers() const -> std::vector<vk::raii::Buffer>;
         [[nodiscard]] auto uniform_buffer_memories() const -> std::vector<vk::raii::DeviceMemory>;
         [[nodiscard]] auto texture() const -> vk::raii::Image;
         [[nodiscard]] auto texture_memory() const -> vk::raii::DeviceMemory;
         [[nodiscard]] auto descriptor_pool() const -> vk::raii::DescriptorPool;
         [[nodiscard]] auto descriptor_sets() const -> std::vector<vk::raii::DescriptorSet>;

         auto recreate_swap_chain() -> void;

         std::uint8_t frame_index_{};
         std::vector<Vertex> const vertices_{
            { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
            { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
            { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
            { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
         };
         std::vector<uint16_t> const indices_{ 0, 1, 3, 2 };

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
         vk::raii::DescriptorSetLayout const descriptor_set_layout_{ descriptor_set_layout() };
         vk::raii::PipelineLayout const pipeline_layout_{ pipeline_layout() };
         vk::raii::Pipeline const pipeline_{ pipeline() };
         vk::raii::CommandPool const command_pool_{ command_pool() };
         std::vector<vk::raii::CommandBuffer> const command_buffers_{ command_buffers() };
         std::vector<vk::raii::Semaphore> const image_available_semaphores_{ semaphores() };
         std::vector<vk::raii::Semaphore> const command_buffer_finished_semaphores_{ semaphores() };
         std::vector<vk::raii::Fence> const presentation_finished_fences_{ fences() };
         vk::raii::Buffer const vertex_buffer_{ vertex_buffer() };
         vk::raii::DeviceMemory const vertex_buffer_memory_{ vertex_buffer_memory() };
         vk::raii::Buffer const index_buffer_{ index_buffer() };
         vk::raii::DeviceMemory const index_buffer_memory_{ index_buffer_memory() };
         std::vector<vk::raii::Buffer> uniform_buffers_{ uniform_buffers() };
         std::vector<vk::raii::DeviceMemory> uniform_buffer_memories_{ uniform_buffer_memories() };
         std::vector<UniformBufferObject*> uniform_buffer_mapped_{};
         vk::raii::Image const texture_{ texture() };
         vk::raii::DeviceMemory const texture_memory_{ texture_memory() };
         vk::raii::DescriptorPool const descriptor_pool_{ descriptor_pool() };
         std::vector<vk::raii::DescriptorSet> const descriptor_sets_{ descriptor_sets() };
   };
}

#endif