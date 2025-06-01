#include "buffer_builder.hpp"

namespace eru
{
   BufferBuilder& BufferBuilder::change_size(vk::DeviceSize const size)
   {
      buffer_info_.size = size;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_usage(vk::BufferUsageFlags const usage)
   {
      buffer_info_.usage = usage;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_sharing_mode(vk::SharingMode const sharing_mode)
   {
      buffer_info_.sharingMode = sharing_mode;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_flags(VmaAllocationCreateFlags const allocation_flags)
   {
      allocation_info_.flags = allocation_flags;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_usage(VmaMemoryUsage const allocation_usage)
   {
      allocation_info_.usage = allocation_usage;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_required_flags(vk::MemoryPropertyFlags const required_flags)
   {
      allocation_info_.requiredFlags = static_cast<VkMemoryPropertyFlags>(required_flags);
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_preferred_flags(vk::MemoryPropertyFlags const preferred_flags)
   {
      allocation_info_.preferredFlags = static_cast<VkMemoryPropertyFlags>(preferred_flags);
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_memory_type_bits(std::uint32_t const memory_type_bits)
   {
      allocation_info_.memoryTypeBits = memory_type_bits;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_pool(VmaPool const pool)
   {
      allocation_info_.pool = pool;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_user_data(void* const user_data)
   {
      allocation_info_.pUserData = user_data;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_priority(float const priority)
   {
      allocation_info_.priority = priority;
      return *this;
   }

   Buffer BufferBuilder::build(Device const& device) const
   {
      VkBuffer buffer;
      VmaAllocation memory;
      vmaCreateBuffer(device.allocator().get(), &static_cast<VkBufferCreateInfo const&>(buffer_info_),
         &allocation_info_, &buffer, &memory, nullptr);

      return { device.allocator().get(), static_cast<vk::Buffer>(buffer), memory };
   }
}