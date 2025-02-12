#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "erupch.hpp"
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

         void run() const;

      private:
         [[nodiscard]] static vk::Instance create_instance();

         [[nodiscard]] vk::SurfaceKHR create_surface() const;

         [[nodiscard]] vk::PhysicalDevice pick_physical_device() const;
         [[nodiscard]] std::uint32_t graphics_queue_index() const;
         [[nodiscard]] std::uint32_t presentation_queue_index() const;
         [[nodiscard]] vk::Device create_device() const;

         [[nodiscard]] vk::SurfaceFormatKHR pick_swap_chain_format() const;
         [[nodiscard]] vk::Extent2D pick_swap_chain_extent() const;
         [[nodiscard]] vk::SwapchainKHR create_swap_chain() const;
         [[nodiscard]] std::vector<vk::ImageView> create_image_views() const;
         [[nodiscard]] vk::ShaderModule create_shader_module(std::vector<std::uint32_t> const& byte_code) const;

         [[nodiscard]] vk::RenderPass create_render_pass() const;
         [[nodiscard]] std::vector<vk::Framebuffer> create_frame_buffers() const;
         [[nodiscard]] vk::PipelineLayout create_pipeline_layout() const;
         [[nodiscard]] vk::Pipeline create_pipeline() const;

         [[nodiscard]] vk::CommandPool create_command_pool() const;
         [[nodiscard]] vk::CommandBuffer create_command_buffer() const;

         void record_command_buffer(vk::CommandBuffer command_buffer, std::uint32_t image_index) const;
         void draw_frame() const;

         unique_pointer<GLFWwindow> const window_{
            glfwCreateWindow(1280, 720, "Vulkan window", nullptr, nullptr),
            glfwDestroyWindow
         };

         vk::Instance const instance_{ create_instance() };

         vk::SurfaceKHR const surface_{ create_surface() };

         vk::PhysicalDevice const physical_device_{ pick_physical_device() };
         std::uint32_t const graphics_queue_index_{ graphics_queue_index() };
         std::uint32_t const presentation_queue_index_{ presentation_queue_index() };
         vk::Device const device_{ create_device() };
         vk::Queue const graphics_queue_{ device_.getQueue(graphics_queue_index_, 0) };
         vk::Queue const presentation_queue_{ device_.getQueue(presentation_queue_index_, 0) };

         vk::SurfaceFormatKHR const swap_chain_format_{ pick_swap_chain_format() };
         vk::Extent2D const swap_chain_extent_{ pick_swap_chain_extent() };
         vk::SwapchainKHR const swap_chain_{ create_swap_chain() };
         std::vector<vk::Image> const swap_chain_images_{ device_.getSwapchainImagesKHR(swap_chain_) };
         std::vector<vk::ImageView> const swap_chain_image_views_{ create_image_views() };

         vk::RenderPass const render_pass_{ create_render_pass() };
         std::vector<vk::Framebuffer> swap_chain_framebuffers_{ create_frame_buffers() };
         vk::PipelineLayout const pipeline_layout_{ create_pipeline_layout() };
         vk::Pipeline const pipeline_{ create_pipeline() };

         vk::CommandPool const command_pool_{ create_command_pool() };
         vk::CommandBuffer const command_buffer_{ create_command_buffer() };
         vk::Semaphore const image_available_semaphore_{ device_.createSemaphore({}) };
         vk::Semaphore const render_finished_semaphore_{ device_.createSemaphore({}) };
         vk::Fence const command_buffer_executed_fence{ device_.createFence({ .flags{ vk::FenceCreateFlagBits::eSignaled } }) };
   };
}

#endif