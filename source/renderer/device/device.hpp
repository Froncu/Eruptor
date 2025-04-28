#ifndef DEVICE_HPP
#define DEVICE_HPP

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
         [[nodiscard]] std::vector<vk::raii::Queue> const& queues() const;

      private:
         Device(vk::raii::PhysicalDevice physical_device, vk::raii::Device device, std::vector<vk::raii::Queue> queues);

         vk::raii::PhysicalDevice physical_device_;
         vk::raii::Device device_;
         std::vector<vk::raii::Queue> queues_;
   };
}

#endif