#ifndef BUFFER_BUILDER_HPP
#define BUFFER_BUILDER_HPP

#include "erupch/erupch.hpp"
#include "renderer/buffer.hpp"
#include "renderer/device.hpp"

namespace eru
{
   class BufferBuilder final
   {
      public:
         BufferBuilder() = default;
         BufferBuilder(BufferBuilder const&) = default;
         BufferBuilder(BufferBuilder&&) = default;

         ~BufferBuilder() = default;

         BufferBuilder& operator=(BufferBuilder const&) = default;
         BufferBuilder& operator=(BufferBuilder&&) = default;

         BufferBuilder& change_size(vk::DeviceSize size);
         BufferBuilder& change_usage(vk::BufferUsageFlags usage);
         BufferBuilder& change_sharing_mode(vk::SharingMode sharing_mode);
         BufferBuilder& change_allocation_flags(VmaAllocationCreateFlags allocation_flags);
         BufferBuilder& change_allocation_usage(VmaMemoryUsage allocation_usage);
         BufferBuilder& change_allocation_required_flags(vk::MemoryPropertyFlags required_flags);
         BufferBuilder& change_allocation_preferred_flags(vk::MemoryPropertyFlags preferred_flags);
         BufferBuilder& change_allocation_memory_type_bits(std::uint32_t memory_type_bits);
         BufferBuilder& change_allocation_pool(VmaPool pool);
         BufferBuilder& change_allocation_user_data(void* user_data);
         BufferBuilder& change_allocation_priority(float priority);

         [[nodiscard]] Buffer build(Device const& device) const;

      private:
         vk::BufferCreateInfo buffer_info_{};
         VmaAllocationCreateInfo allocation_info_{};
   };
}

#endif