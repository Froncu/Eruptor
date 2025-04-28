#ifndef DEVICE_BUILDER_HPP
#define DEVICE_BUILDER_HPP

#include "context/context.hpp"
#include "device.hpp"
#include "erupch/erupch.hpp"

namespace eru
{
   class DeviceBuilder final
   {
      using RequiredQueueInfo = std::pair<vk::QueueFlags, vk::SurfaceKHR>;

      struct QueueInfo final
      {
         std::unordered_map<std::uint32_t, std::uint32_t> family_create_counts{};
         std::vector<std::uint32_t> retrieval_infos{};
      };

      public:
         explicit DeviceBuilder(Context const& context);
         DeviceBuilder(DeviceBuilder const&) = delete;
         DeviceBuilder(DeviceBuilder&&) = delete;

         ~DeviceBuilder() = default;

         DeviceBuilder& operator=(DeviceBuilder const&) = delete;
         DeviceBuilder& operator=(DeviceBuilder&&) = delete;

         DeviceBuilder& enable_extension(std::string extension_name);
         DeviceBuilder& enable_extensions(std::span<std::string> extension_names);
         DeviceBuilder& enable_features(vk::PhysicalDeviceFeatures const& features);
         DeviceBuilder& add_queues(RequiredQueueInfo const& info, std::uint32_t count = 1);
         DeviceBuilder& add_queues(vk::QueueFlags flags, std::uint32_t count = 1);
         DeviceBuilder& add_queues(vk::raii::SurfaceKHR surface, std::uint32_t count = 1);

         [[nodiscard]] Device build();

      private:
         [[nodiscard]] vk::raii::PhysicalDevice pick_physical_device();
         [[nodiscard]] vk::raii::Device create_device(vk::raii::PhysicalDevice const& physical_device);
         [[nodiscard]] std::vector<vk::raii::Queue> retrieve_queues(vk::raii::PhysicalDevice const& physical_device,
            vk::raii::Device const& device);

         Context const& context_;

         std::set<std::string> extension_names_{};
         vk::PhysicalDeviceFeatures features_{};
         std::vector<RequiredQueueInfo> required_queues_{};
         std::unordered_map<VkPhysicalDevice, QueueInfo> queue_infos_{};
   };
}

#endif