#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "eruptor/pass_key.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Locator;

   class Context
   {
      public:
         explicit Context(PassKey<Locator>);
         Context(Context const&) = delete;
         Context(Context&&) = delete;

         ~Context() = default;

         auto operator=(Context const&) -> Context& = delete;
         auto operator=(Context&&) -> Context& = delete;

         [[nodiscard]] auto allocate_memory(vk::MemoryRequirements const& requirements, vk::MemoryPropertyFlags properties) const -> vk::raii::DeviceMemory;
         [[nodiscard]] auto create_semaphores(std::uint32_t count = 1) const -> std::vector<vk::raii::Semaphore>;

         vk::raii::Context const vulkan_context{};
         vk::raii::Instance const instance{ create_instance() };
         vk::raii::DebugUtilsMessengerEXT const debug_messenger{ create_debug_messenger() };
         vk::raii::PhysicalDevice const physical_device{ pick_physical_device() };
         std::uint32_t const queue_family_index{ pick_queue_family_index() };
         vk::raii::Device const device{ create_device() };
         vk::raii::Queue const queue{ retrieve_queue() };
         vk::raii::CommandPool const command_pool{ create_command_pool() };

      private:
         [[nodiscard]] auto create_instance() const -> vk::raii::Instance;
         [[nodiscard]] auto create_debug_messenger() const -> vk::raii::DebugUtilsMessengerEXT;
         [[nodiscard]] auto pick_physical_device() const -> vk::raii::PhysicalDevice;
         [[nodiscard]] auto pick_queue_family_index() const -> std::uint32_t;
         [[nodiscard]] auto create_device() const -> vk::raii::Device;
         [[nodiscard]] auto retrieve_queue() const -> vk::raii::Queue;
         [[nodiscard]] auto create_command_pool() const -> vk::raii::CommandPool;
   };
}

#endif