#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "constants.hpp"
#include "erupch/erupch.hpp"
#include "math/vertex.hpp"
#include "utility/unique_pointer.hpp"

namespace eru
{
   class application final
   {
      public:
         application() = default;
         application(application const&) = delete;
         application(application&&) = delete;

         ~application();

         application& operator=(application const&) = delete;
         application& operator=(application&&) = delete;

         void run();

      private:
         static bool constexpr USE_VALIDATION_LAYERS{ constants::DEBUG };

         [[nodiscard]] static vk::Instance create_instance();
         [[nodiscard]] vk::DebugUtilsMessengerEXT create_debug_callback_messenger() const;

         [[nodiscard]] vk::SurfaceKHR create_surface() const;

         [[nodiscard]] vk::PhysicalDevice pick_physical_device() const;
         [[nodiscard]] std::uint32_t graphics_queue_family_index() const;
         [[nodiscard]] std::uint32_t presentation_queue_family_index() const;
         [[nodiscard]] vk::Device create_device() const;

         [[nodiscard]] vk::SurfaceFormatKHR pick_swap_chain_format() const;
         [[nodiscard]] vk::Extent2D pick_swap_chain_extent() const;
         [[nodiscard]] vk::SwapchainKHR create_swap_chain() const;
         [[nodiscard]] std::vector<vk::ImageView> create_image_views() const;
         [[nodiscard]] vk::ShaderModule create_shader_module(std::vector<std::uint32_t> const& byte_code) const;

         [[nodiscard]] vk::RenderPass create_render_pass() const;
         [[nodiscard]] std::vector<vk::Framebuffer> create_frame_buffers() const;
         [[nodiscard]] vk::DescriptorSetLayout create_descriptor_set_layout() const;
         [[nodiscard]] vk::PipelineLayout create_pipeline_layout() const;
         [[nodiscard]] vk::Pipeline create_pipeline() const;

         [[nodiscard]] vk::CommandPool create_command_pool() const;
         [[nodiscard]] VmaAllocator create_allocator() const;
         [[nodiscard]] std::pair<vk::Buffer, VmaAllocation> create_buffer(vk::DeviceSize size,
            vk::BufferUsageFlags usage, VmaAllocationCreateFlags allocation_flags,
            vk::MemoryPropertyFlags required_properties, vk::MemoryPropertyFlags preferred_properties) const;
         void copy_buffer(vk::Buffer source_buffer, vk::Buffer target_buffer, vk::DeviceSize size) const;
         [[nodiscard]] std::pair<vk::Buffer, VmaAllocation> create_vertex_buffer() const;
         [[nodiscard]] std::pair<vk::Buffer, VmaAllocation> create_index_buffer() const;
         [[nodiscard]] std::vector<std::pair<vk::Buffer, VmaAllocation>> create_uniform_buffers() const;
         [[nodiscard]] vk::DescriptorPool create_descriptor_pool() const;
         [[nodiscard]] std::vector<vk::DescriptorSet> create_descriptor_sets() const;
         [[nodiscard]] std::vector<vk::CommandBuffer> create_command_buffers() const;
         [[nodiscard]] std::vector<vk::Semaphore> create_semaphores() const;
         [[nodiscard]] std::vector<vk::Fence> create_fences() const;

         void record_command_buffer(vk::CommandBuffer command_buffer, std::uint32_t image_index) const;
         void draw_frame();
         void update_uniform_buffer(std::size_t current_frame) const;

         unique_pointer<SDL_Window> const window_{
            []
            {
               if (not SDL_Init(SDL_INIT_VIDEO))
                  throw std::runtime_error(std::format("failed to initialize the video subsystem! -> {}", SDL_GetError()));

               return SDL_CreateWindow("Eruptor", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
            }(),
            [](SDL_Window* const window)
            {
               SDL_DestroyWindow(window);
               SDL_QuitSubSystem(SDL_INIT_VIDEO);
            }
         };

         vk::Instance const instance_{ create_instance() };
         vk::DispatchLoaderDynamic dispatch_loader_dynamic_{ instance_, vkGetInstanceProcAddr };

         vk::DebugUtilsMessengerEXT debug_messenger_{ USE_VALIDATION_LAYERS ? create_debug_callback_messenger() : nullptr };

         vk::SurfaceKHR const surface_{ create_surface() };

         vk::PhysicalDevice const physical_device_{ pick_physical_device() };
         std::uint32_t const graphics_queue_family_index_{ graphics_queue_family_index() };
         std::uint32_t const presentation_queue_family_index_{ presentation_queue_family_index() };
         vk::Device const device_{ create_device() };
         vk::Queue const graphics_queue_{ device_.getQueue(graphics_queue_family_index_, 0) };
         vk::Queue const presentation_queue_{ device_.getQueue(presentation_queue_family_index_, 0) };

         vk::SurfaceFormatKHR swap_chain_format_{ pick_swap_chain_format() };
         vk::Extent2D swap_chain_extent_{ pick_swap_chain_extent() };
         vk::SwapchainKHR swap_chain_{ create_swap_chain() };
         std::vector<vk::Image> swap_chain_images_{ device_.getSwapchainImagesKHR(swap_chain_) };
         std::vector<vk::ImageView> swap_chain_image_views_{ create_image_views() };

         vk::RenderPass const render_pass_{ create_render_pass() };
         std::vector<vk::Framebuffer> swap_chain_framebuffers_{ create_frame_buffers() };
         vk::DescriptorSetLayout const descriptor_set_layout_{ create_descriptor_set_layout() };
         vk::PipelineLayout const pipeline_layout_{ create_pipeline_layout() };
         vk::Pipeline const pipeline_{ create_pipeline() };

         vk::CommandPool const command_pool_{ create_command_pool() };
         static std::uint32_t constexpr FRAMES_IN_FLIGHT{ 2 };
         std::size_t current_frame_{};
         std::vector<Vertex> const vertices_{
            { { -0.5f, 0.0f, 0.5f }, { 1.0f, 0.0f, 0.0f } },
            { { 0.5f, 0.0f, 0.5f }, { 1.0f, 0.25f, 0.0f } },
            { { 0.5f, 0.0f, -0.5f }, { 1.0f, 1.0f, 0.0f } },
            { { -0.5f, 0.0f, -0.5f }, { 1.0f, 0.25f, 0.0f } }
         };
         std::vector<std::uint16_t> const indices_{ 0, 1, 3, 2 };
         VmaAllocator const allocator_{ create_allocator() };
         std::pair<vk::Buffer, VmaAllocation> const vertex_buffer_{ create_vertex_buffer() };
         std::pair<vk::Buffer, VmaAllocation> const index_buffer_{ create_index_buffer() };
         std::vector<std::pair<vk::Buffer, VmaAllocation>> const uniform_buffers_{ create_uniform_buffers() };
         vk::DescriptorPool const descriptor_pool_{ create_descriptor_pool() };
         std::vector<vk::DescriptorSet> const descriptor_sets_{ create_descriptor_sets() };
         std::vector<vk::CommandBuffer> const command_buffers_{ create_command_buffers() };
         std::vector<vk::Semaphore> const image_available_semaphores_{ create_semaphores() };
         std::vector<vk::Semaphore> const render_finished_semaphores_{ create_semaphores() };
         std::vector<vk::Fence> const command_buffer_executed_fences_{ create_fences() };

         void recreate_swapchain();
   };
}

#endif