#include "device.hpp"

namespace eru
{
   Device::Device(vk::raii::PhysicalDevice physical_device, vk::raii::Device device, std::vector<vk::raii::Queue> queues)
      : physical_device_{ std::move(physical_device) }
      , device_{ std::move(device) }
      , queues_{ std::move(queues) }
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

   std::vector<vk::raii::Queue> const& Device::queues() const
   {
      return queues_;
   }
}