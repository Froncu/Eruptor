#include "buffer_builder.hpp"

namespace eru
{
   BufferBuilder& BufferBuilder::change_buffer_create_info(vk::BufferCreateInfo const& buffer_create_info)
   {
      buffer_create_info_ = buffer_create_info;
      return *this;
   }

   BufferBuilder& BufferBuilder::change_allocation_create_info(VmaAllocationCreateInfo const& allocation_create_info)
   {
      allocation_create_info_ = allocation_create_info;
      return *this;
   }

   Buffer BufferBuilder::build(Device const& device) const
   {
      VkBuffer buffer;
      VmaAllocation memory;
      vmaCreateBuffer(device.allocator().get(), &static_cast<VkBufferCreateInfo const&>(buffer_create_info_),
         &allocation_create_info_, &buffer, &memory, nullptr);

      return { device.allocator().get(), static_cast<vk::Buffer>(buffer), memory };
   }
}