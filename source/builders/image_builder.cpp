#include "image_builder.hpp"

namespace eru
{
   ImageBuilder& ImageBuilder::change_create_info(vk::ImageCreateInfo const& create_info)
   {
      create_info_ = create_info;
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

      return { Image::OwnedImage{ device.allocator().get(), static_cast<vk::Image>(image), memory }, create_info_ };
   }
}