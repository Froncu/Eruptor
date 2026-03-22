#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Buffer final : public vk::Buffer
   {
      public:
         ERU_API Buffer(VmaAllocator allocator, vk::Buffer buffer, VmaAllocation allocation);
         Buffer(Buffer const&) = delete;
         Buffer(Buffer&&) = delete;

         ERU_API ~Buffer();

         Buffer& operator=(Buffer const&) = delete;
         Buffer& operator=(Buffer&&) = delete;

      private:
         VmaAllocator allocator_;
         VmaAllocation allocation_;
   };
}

#endif