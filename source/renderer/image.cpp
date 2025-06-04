#include "image.hpp"
#include "utility/exception.hpp"

namespace eru
{
   Image::Image(std::variant<OwnedImage, vk::Image> image, vk::ImageCreateInfo info)
      : image_{ std::move(image) }
      , info_{ std::move(info) }
   {
   }

   void Image::transition_layout(Device const& device, vk::ImageLayout new_layout)
   {
      DeviceQueue const& queue{ device.queues().front() };
      vk::raii::CommandBuffer const command_buffer{
         std::move(device.device().allocateCommandBuffers({
            .commandPool{ *device.command_pool(queue) },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ 1 }
         }).front())
      };

      vk::AccessFlags source_access_mask;
      vk::AccessFlags destination_access_mask;
      vk::ImageAspectFlags image_aspect_flags;
      vk::PipelineStageFlags source_stage_mask;
      vk::PipelineStageFlags destination_stage_mask;
      if (info_.initialLayout == vk::ImageLayout::eUndefined and
         new_layout == vk::ImageLayout::eTransferDstOptimal)
      {
         source_access_mask = vk::AccessFlagBits::eNone;
         destination_access_mask = vk::AccessFlagBits::eTransferWrite;

         image_aspect_flags = vk::ImageAspectFlagBits::eColor;

         source_stage_mask = vk::PipelineStageFlagBits::eTopOfPipe;
         destination_stage_mask = vk::PipelineStageFlagBits::eTransfer;
      }
      else if (info_.initialLayout == vk::ImageLayout::eTransferDstOptimal and
         new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
      {
         source_access_mask = vk::AccessFlagBits::eTransferWrite;
         destination_access_mask = vk::AccessFlagBits::eShaderRead;

         image_aspect_flags = vk::ImageAspectFlagBits::eColor;

         source_stage_mask = vk::PipelineStageFlagBits::eTransfer;
         destination_stage_mask = vk::PipelineStageFlagBits::eFragmentShader;
      }
      else if (info_.initialLayout == vk::ImageLayout::eUndefined &&
         new_layout == vk::ImageLayout::eDepthStencilReadOnlyOptimal)
      {
         source_access_mask = vk::AccessFlagBits::eNone;
         destination_access_mask = vk::AccessFlagBits::eDepthStencilAttachmentRead;

         image_aspect_flags = vk::ImageAspectFlagBits::eDepth;

         source_stage_mask = vk::PipelineStageFlagBits::eTopOfPipe;
         destination_stage_mask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
      }
      else
         exception("unsupported layout transition!");

      vk::ImageMemoryBarrier const image_memory_barrier{
         .srcAccessMask{ source_access_mask },
         .dstAccessMask{ destination_access_mask },
         .oldLayout{ info_.initialLayout },
         .newLayout{ new_layout },
         .srcQueueFamilyIndex{ vk::QueueFamilyIgnored },
         .dstQueueFamilyIndex{ vk::QueueFamilyIgnored },
         .image{ image() },
         .subresourceRange{
            .aspectMask{ image_aspect_flags },
            .baseMipLevel{ 0 },
            .levelCount{ 1 },
            .baseArrayLayer{ 0 },
            .layerCount{ 1 }
         }
      };

      command_buffer.begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });

      command_buffer.pipelineBarrier(source_stage_mask, destination_stage_mask, {}, {}, {}, { image_memory_barrier });

      command_buffer.end();

      queue.queue().submit({
         {
            .commandBufferCount{ 1 },
            .pCommandBuffers{ &*command_buffer }
         }
      });
      queue.queue().waitIdle();

      info_.initialLayout = new_layout;
   }

   vk::Image Image::image() const
   {
      if (std::holds_alternative<OwnedImage>(image_))
         return std::get<OwnedImage>(image_).image;

      return std::get<vk::Image>(image_);
   }

   vk::ImageCreateInfo const& Image::info() const
   {
      return info_;
   }
}