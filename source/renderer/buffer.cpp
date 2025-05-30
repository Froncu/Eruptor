#include "buffer.hpp"

namespace eru
{
   Buffer::Buffer(VmaAllocator const allocator, vk::Buffer const buffer, VmaAllocation const allocation)
      : allocator_{ allocator }
      , buffer_{ buffer }
      , allocation_{ allocation }
   {
   }

   Buffer::Buffer(Buffer&& other) noexcept
      : allocator_{ other.allocator_ }
      , buffer_{ other.buffer_ }
      , allocation_{ other.allocation_ }
   {
      other.allocator_ = nullptr;
      other.buffer_ = nullptr;
      other.allocation_ = nullptr;
   }

   Buffer::~Buffer()
   {
      if (allocator_)
         vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer_), allocation_);
   }

   Buffer& Buffer::operator=(Buffer&& other) noexcept
   {
      if (this == &other)
         return *this;

      allocator_ = other.allocator_;
      buffer_ = other.buffer_;
      allocation_ = other.allocation_;

      other.allocator_ = nullptr;
      other.buffer_ = nullptr;
      other.allocation_ = nullptr;

      return *this;
   }

   VmaAllocator Buffer::allocator() const
   {
      return allocator_;
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