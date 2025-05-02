#include "device.hpp"

namespace eru
{
   Device::Device(vk::raii::PhysicalDevice physical_device, vk::raii::Device device, std::vector<DeviceQueue> queues,
      std::unordered_map<std::uint32_t, vk::raii::CommandPool> command_pools, UniquePointer<VmaAllocator_T> allocator)
      : physical_device_{ std::move(physical_device) }
      , device_{ std::move(device) }
      , queues_{ std::move(queues) }
      , command_pools_{ std::move(command_pools) }
      , allocator_{ std::move(allocator) }
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

   vk::raii::CommandPool const& Device::command_pool(DeviceQueue const& device_queue) const
   {
      return command_pools_.at(device_queue.family_index());
   }

   UniquePointer<VmaAllocator_T> const& Device::allocator() const
   {
      return allocator_;
   }
}