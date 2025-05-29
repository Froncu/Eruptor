#include "buffer.hpp"

namespace eru
{
   Buffer::Buffer(VmaAllocator const allocator, vk::Buffer buffer, VmaAllocation const allocation)
      : allocator_{ allocator }
      , buffer_{ std::move(buffer) }
      , allocation_{ allocation }
   {
   }

   Buffer::~Buffer()
   {
      vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer_), allocation_);
   }

   vk::Buffer Buffer::buffer() const
   {
      return buffer_;
   }

   VmaAllocation Buffer::allocation() const
   {
      return allocation_;
   }
}