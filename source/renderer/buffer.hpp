#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "device.hpp"
#include "image.hpp"

namespace eru
{
   class Buffer final
   {
      friend class BufferBuilder;

      public:
         Buffer() = default;
         Buffer(Buffer const&) = delete;
         Buffer(Buffer&& other) noexcept;

         ~Buffer();

         Buffer& operator=(Buffer const&) = delete;
         Buffer& operator=(Buffer&& other) noexcept;

         void upload(void const* data, std::size_t size);
         void copy(Device const& device, Buffer const& target, vk::DeviceSize size) const;
         void copy(Device const& device, Image const& target, vk::Extent3D extent) const;

         [[nodiscard]] VmaAllocator allocator() const;
         [[nodiscard]] vk::Buffer buffer() const;
         [[nodiscard]] VmaAllocation allocation() const;

      private:
         Buffer(VmaAllocator allocator, vk::Buffer buffer, VmaAllocation allocation);

         VmaAllocator allocator_{};
         vk::Buffer buffer_{};
         VmaAllocation allocation_{};
   };
}

#endif