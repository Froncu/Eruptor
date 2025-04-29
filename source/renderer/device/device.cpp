#include "device.hpp"

namespace eru
{
   Device::Device(vk::raii::PhysicalDevice physical_device, vk::raii::Device device, std::vector<DeviceQueue> queues,
      std::unordered_map<std::uint32_t, vk::raii::CommandPool> command_pools)
      : physical_device_{ std::move(physical_device) }
      , device_{ std::move(device) }
      , queues_{ std::move(queues) }
      , command_pools_{ std::move(command_pools) }
   {
   }

   vk::raii::PhysicalDevice const& Device::physical_device() const
   {
      return physical_device_;
   }

   vk::raii::Device const& Device::device() const
   {
      return device_;
   }

   std::vector<DeviceQueue> const& Device::queues() const
   {
      return queues_;
   }
}