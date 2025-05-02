#ifndef DEVICE_BUILDER_HPP
#define DEVICE_BUILDER_HPP

#include "erupch/erupch.hpp"
#include "renderer/device.hpp"
#include "renderer/device_queue.hpp"

namespace eru
{
   class DeviceBuilder final
   {
      struct QueueInfo final
      {
         std::unordered_map<std::uint32_t, std::uint32_t> family_create_counts{};
         std::vector<std::uint32_t> retrieval_infos{};
      };

      public:
         using RequiredQueueInfo = std::pair<vk::QueueFlags, vk::SurfaceKHR>;

         DeviceBuilder() = default;
         DeviceBuilder(DeviceBuilder const&) = delete;
         DeviceBuilder(DeviceBuilder&&) = delete;

         ~DeviceBuilder() = default;

         DeviceBuilder& operator=(DeviceBuilder const&) = delete;
         DeviceBuilder& operator=(DeviceBuilder&&) = delete;

         DeviceBuilder& enable_extension(std::string extension_name);
         DeviceBuilder& enable_extensions(std::vector<std::string> extension_names);
         DeviceBuilder& enable_features(vk::PhysicalDeviceFeatures const& features);
         DeviceBuilder& add_queues(RequiredQueueInfo const& info, std::uint32_t count = 1);
         DeviceBuilder& add_queues(vk::QueueFlags flags, std::uint32_t count = 1);
         DeviceBuilder& add_queues(vk::raii::SurfaceKHR surface, std::uint32_t count = 1);

         [[nodiscard]] Device build();

      private:
         [[nodiscard]] static UniquePointer<VmaAllocator_T> create_allocator(vk::raii::PhysicalDevice const& physical_device,
            vk::raii::Device const& device);

         [[nodiscard]] vk::raii::PhysicalDevice pick_physical_device();
         [[nodiscard]] vk::raii::Device create_device(vk::raii::PhysicalDevice const& physical_device);
         [[nodiscard]] std::vector<DeviceQueue> retrieve_queues(vk::raii::PhysicalDevice const& physical_device,
            vk::raii::Device const& device);
         [[nodiscard]] std::unordered_map<std::uint32_t, vk::raii::CommandPool> create_command_pools(
            vk::raii::PhysicalDevice const& physical_device, vk::raii::Device const& device);

         bool dynamic_rendering_{};
         std::set<std::string> extension_names_{};
         vk::PhysicalDeviceFeatures features_{};
         std::vector<RequiredQueueInfo> required_queues_{};
         std::unordered_map<VkPhysicalDevice, QueueInfo> queue_infos_{};
   };
}

#endif