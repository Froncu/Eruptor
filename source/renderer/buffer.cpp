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

      if (allocator_)
         vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer_), allocation_);

      allocator_ = other.allocator_;
      buffer_ = other.buffer_;
      allocation_ = other.allocation_;

      other.allocator_ = nullptr;
      other.buffer_ = nullptr;
      other.allocation_ = nullptr;

      return *this;
   }

   void Buffer::upload(void const* const data, std::size_t const size)
   {
      VmaAllocationInfo allocation_info;
      vmaGetAllocationInfo(allocator_, allocation_, &allocation_info);
      std::memcpy(allocation_info.pMappedData, data, size);
   }

   void Buffer::copy(Device const& device, Buffer const& target, vk::DeviceSize size) const
   {
      DeviceQueue const& queue{ device.queues().front() };
      vk::raii::CommandBuffer const command_buffer{
         std::move(device.device().allocateCommandBuffers({
            .commandPool{ *device.command_pool(queue) },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ 1 }
         }).front())
      };

      command_buffer.begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });

      command_buffer.copyBuffer(buffer_, target.buffer(), {
         {
            .size{ size }
         }
      });

      command_buffer.end();

      queue.queue().submit({
         {
            .commandBufferCount{ 1 },
            .pCommandBuffers{ &*command_buffer }
         }
      });
      queue.queue().waitIdle();
   }

   void Buffer::copy(Device const& device, Image const& target, vk::Extent3D extent) const
   {
      DeviceQueue const& queue{ device.queues().front() };
      vk::CommandBuffer const command_buffer{
         device.device().allocateCommandBuffers({
            .commandPool{ *device.command_pool(queue) },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ 1 }
         }).front()
      };

      command_buffer.copyBufferToImage(buffer_, target.image(), vk::ImageLayout::eTransferDstOptimal, {
         {
            .imageSubresource{
               .aspectMask{ vk::ImageAspectFlagBits::eColor },
               .layerCount{ 1 }
            },
            .imageExtent{ extent }
         }
      });

      command_buffer.end();

      queue.queue().submit({
         {
            .commandBufferCount{ 1 },
            .pCommandBuffers{ &command_buffer }
         }
      });
      queue.queue().waitIdle();
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