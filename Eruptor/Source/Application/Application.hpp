#if not defined APPLICATION_HPP
#define APPLICATION_HPP

#include "erupch.hpp"

#include "utility/unique_pointer.hpp"

namespace eru
{
   class application final
   {
   public:
      application();
      application(application const&) = delete;
      application(application&&) = delete;

      ~application();

      application& operator=(application const&) = delete;
      application& operator=(application&&) = delete;

      void run() const;

   private:
      [[nodiscard]] vk::Instance create_instance() const;
      [[nodiscard]] vk::PhysicalDevice pick_physical_device() const;
      [[nodiscard]] std::optional<std::uint32_t> graphics_queue_index() const;
      [[nodiscard]] vk::Device create_device() const;

      unique_pointer<GLFWwindow> const window_{ glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr), glfwDestroyWindow };
      vk::Instance const instance_{ create_instance() };
      vk::PhysicalDevice const physical_device_{ pick_physical_device() };
      std::uint32_t const graphics_queue_index_{ graphics_queue_index().value() };
      vk::Device const device_{ create_device() };
      vk::Queue const graphics_queue_{ device_.getQueue(graphics_queue_index_, 0) };
   };
}

#endif