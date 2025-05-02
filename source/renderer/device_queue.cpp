#include "device_queue.hpp"

namespace eru
{
   DeviceQueue::DeviceQueue(std::uint32_t const family_index, vk::raii::Queue queue)
      : family_index_{ family_index }
      , queue_{ std::move(queue) }
   {
   }

   std::uint32_t DeviceQueue::family_index() const
   {
      return family_index_;
   }

   vk::raii::Queue const& DeviceQueue::queue() const
   {
      return queue_;
   }
}