#ifndef DEVICE_QUEUE_HPP
#define DEVICE_QUEUE_HPP

namespace eru
{
   class DeviceQueue
   {
      friend class DeviceBuilder;

      public:
         DeviceQueue(DeviceQueue const&) = delete;
         DeviceQueue(DeviceQueue&&) = default;

         ~DeviceQueue() = default;

         DeviceQueue& operator=(DeviceQueue const&) = delete;
         DeviceQueue& operator=(DeviceQueue&&) = default;

         [[nodiscard]] std::uint32_t family_index() const;
         [[nodiscard]] vk::raii::Queue const& queue() const;

      private:
         DeviceQueue(std::uint32_t family_index, vk::raii::Queue queue);

         std::uint32_t family_index_;
         vk::raii::Queue queue_;
   };
}

#endif