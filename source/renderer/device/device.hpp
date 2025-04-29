#ifndef DEVICE_HPP
#define DEVICE_HPP
#include "device_queue.hpp"

namespace eru
{
   class Device final
   {
      friend class DeviceBuilder;

      public:
         Device(Device const&) = delete;
         Device(Device&&) = delete;

         ~Device() = default;

         Device& operator=(Device const&) = delete;
         Device& operator=(Device&&) = delete;

         [[nodiscard]] vk::raii::PhysicalDevice const& physical_device() const;
         [[nodiscard]] vk::raii::Device const& device() const;
         [[nodiscard]] std::vector<DeviceQueue> const& queues() const;

      private:
         Device(vk::raii::PhysicalDevice physical_device, vk::raii::Device device, std::vector<DeviceQueue> queues,
            std::unordered_map<std::uint32_t, vk::raii::CommandPool> command_pools);

         vk::raii::PhysicalDevice physical_device_;
         vk::raii::Device device_;
         std::vector<DeviceQueue> queues_;
         std::unordered_map<std::uint32_t, vk::raii::CommandPool> command_pools_;
   };
}

#endif