#include "eruptor/buffer.hpp"

namespace eru
{
   Buffer::Buffer(VmaAllocator const allocator, vk::Buffer const buffer, VmaAllocation const allocation)
      : vk::Buffer{ buffer }
      , allocator_{ allocator }
      , allocation_{ allocation }
   {
   }

   Buffer::~Buffer()
   {
      vmaDestroyBuffer(allocator_, static_cast<vk::Buffer>(*this), allocation_);
   }
}