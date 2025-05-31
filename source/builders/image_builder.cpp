#include "image_builder.hpp"

namespace eru
{
   ImageBuilder& ImageBuilder::change_type(vk::ImageType const type)
   {
      create_info_.imageType = type;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_format(vk::Format const format)
   {
      create_info_.format = format;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_extent(vk::Extent3D const extent)
   {
      create_info_.extent = extent;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_extent(vk::Extent2D const extent)
   {
      return change_extent({
         .width{ extent.width },
         .height{ extent.height },
         .depth{ 1 }
      });
   }

   ImageBuilder& ImageBuilder::change_mip_levels(std::uint32_t const mip_levels)
   {
      create_info_.mipLevels = mip_levels;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_array_layers(std::uint32_t const array_layers)
   {
      create_info_.arrayLayers = array_layers;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_samples(vk::SampleCountFlagBits const samples)
   {
      create_info_.samples = samples;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_tiling(vk::ImageTiling const tiling)
   {
      create_info_.tiling = tiling;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_usage(vk::ImageUsageFlags const usage)
   {
      create_info_.usage = usage;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_sharing_mode(vk::SharingMode const sharing_mode)
   {
      create_info_.sharingMode = sharing_mode;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_initial_layout(vk::ImageLayout const initial_layout)
   {
      create_info_.initialLayout = initial_layout;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_flags(VmaAllocationCreateFlags const allocation_flags)
   {
      allocation_info_.flags = allocation_flags;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_usage(VmaMemoryUsage const allocation_usage)
   {
      allocation_info_.usage = allocation_usage;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_required_flags(vk::MemoryPropertyFlags const required_flags)
   {
      allocation_info_.requiredFlags = static_cast<VkMemoryPropertyFlags>(required_flags);
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_preferred_flags(vk::MemoryPropertyFlags const preferred_flags)
   {
      allocation_info_.preferredFlags = static_cast<VkMemoryPropertyFlags>(preferred_flags);
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_memory_type_bits(std::uint32_t const memory_type_bits)
   {
      allocation_info_.memoryTypeBits = memory_type_bits;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_pool(VmaPool const pool)
   {
      allocation_info_.pool = pool;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_user_data(void* const user_data)
   {
      allocation_info_.pUserData = user_data;
      return *this;
   }

   ImageBuilder& ImageBuilder::change_allocation_priority(float const priority)
   {
      allocation_info_.priority = priority;
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