#ifndef BUFFER_HPP
#define BUFFER_HPP

namespace eru
{
   class Buffer final
   {
      friend class BufferBuilder;

      public:
         Buffer(Buffer const&) = delete;
         Buffer(Buffer&&) = default;

         ~Buffer();

         Buffer& operator=(Buffer const&) = delete;
         Buffer& operator=(Buffer&&) = default;

         [[nodiscard]] vk::Buffer buffer() const;
         [[nodiscard]] VmaAllocation allocation() const;

      private:
         Buffer(VmaAllocator allocator, vk::Buffer buffer, VmaAllocation allocation);

         VmaAllocator allocator_;
         vk::Buffer buffer_;
         VmaAllocation allocation_;
   };
}

#endif