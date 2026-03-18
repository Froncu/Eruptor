#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"
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

      public:
         Application(Application const&) = delete;
         Application(Application&&) noexcept = delete;

         virtual ~Application() = default;

         Application& operator=(Application const&) = delete;
         Application& operator=(Application&&) = delete;

         [[nodiscard]] virtual bool tick() = 0;
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
         [[nodiscard]] vk::raii::SwapchainKHR swap_chain() const;
         [[nodiscard]] std::vector<vk::raii::ImageView> swap_chain_image_views() const;
         [[nodiscard]] vk::raii::PipelineLayout pipeline_layout() const;
         [[nodiscard]] vk::raii::Pipeline pipeline() const;

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
         vk::raii::SwapchainKHR const swap_chain_{ swap_chain() };
         std::vector<vk::Image> const swap_chain_images_{ swap_chain_.getImages() };
         std::vector<vk::raii::ImageView> const swap_chain_image_views_{ swap_chain_image_views() };
         vk::raii::PipelineLayout const pipeline_layout_{ pipeline_layout() };
         vk::raii::Pipeline const pipeline_{ pipeline() };
   };
}

#endif