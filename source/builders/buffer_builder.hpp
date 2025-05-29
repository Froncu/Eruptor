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
         BufferBuilder(BufferBuilder const&) = delete;
         BufferBuilder(BufferBuilder&&) = delete;

         ~BufferBuilder() = default;

         BufferBuilder& operator=(BufferBuilder const&) = delete;
         BufferBuilder& operator=(BufferBuilder&&) = delete;

         BufferBuilder& change_buffer_create_info(vk::BufferCreateInfo const& buffer_create_info);
         BufferBuilder& change_allocation_create_info(VmaAllocationCreateInfo const& allocation_create_info);

         [[nodiscard]] Buffer build(Device const& device) const;

      private:
         vk::BufferCreateInfo buffer_create_info_{};
         VmaAllocationCreateInfo allocation_create_info_{};
   };
}

#endif