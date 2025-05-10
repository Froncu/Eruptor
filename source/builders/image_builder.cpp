#include "image_builder.hpp"

namespace eru
{
   ImageBuilder& ImageBuilder::change_create_info(vk::ImageCreateInfo const& create_info,
      vk::ImageAspectFlags const identical_aspect_flags)
   {
      create_info_ = create_info;
      identical_aspect_flags_ = identical_aspect_flags;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_info(VmaAllocationCreateInfo const& allocation_info)
   {
      allocation_info_ = allocation_info;
      return *this;
   }

   Image ImageBuilder::build(Device const& device) const
   {
      VkImage image;
      VmaAllocation memory;
      vmaCreateImage(device.allocator().get(), &static_cast<VkImageCreateInfo const&>(create_info_), &allocation_info_,
         &image, &memory, nullptr);

      return {
         Image::OwnedImage{ { device.device(), image }, memory },
         device.device().createImageView({
            .image{ image },
            .viewType{ static_cast<vk::ImageViewType>(create_info_.imageType) },
            .format{ create_info_.format },
            .components{
               .r{ vk::ComponentSwizzle::eIdentity },
               .g{ vk::ComponentSwizzle::eIdentity },
               .b{ vk::ComponentSwizzle::eIdentity },
               .a{ vk::ComponentSwizzle::eIdentity }
            },
            .subresourceRange{
               .aspectMask{ identical_aspect_flags_ },
               .levelCount{ create_info_.mipLevels },
               .layerCount{ create_info_.arrayLayers }
            }
         }),
         create_info_.format,
         create_info_.initialLayout
      };
   }
}