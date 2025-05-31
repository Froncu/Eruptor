#ifndef IMAGE_BUILDER_HPP
#define IMAGE_BUILDER_HPP

#include "erupch/erupch.hpp"
#include "renderer/device.hpp"
#include "renderer/image.hpp"

namespace eru
{
   class ImageBuilder final
   {
      public:
         ImageBuilder() = default;
         ImageBuilder(ImageBuilder const&) = default;
         ImageBuilder(ImageBuilder&&) = default;

         ~ImageBuilder() = default;

         ImageBuilder& operator=(ImageBuilder const&) = default;
         ImageBuilder& operator=(ImageBuilder&&) = default;

         ImageBuilder& change_type(vk::ImageType type);
         ImageBuilder& change_format(vk::Format format);
         ImageBuilder& change_extent(vk::Extent3D extent);
         ImageBuilder& change_extent(vk::Extent2D extent);
         ImageBuilder& change_mip_levels(std::uint32_t mip_levels);
         ImageBuilder& change_array_layers(std::uint32_t array_layers);
         ImageBuilder& change_samples(vk::SampleCountFlagBits samples);
         ImageBuilder& change_tiling(vk::ImageTiling tiling);
         ImageBuilder& change_usage(vk::ImageUsageFlags usage);
         ImageBuilder& change_sharing_mode(vk::SharingMode sharing_mode);
         ImageBuilder& change_initial_layout(vk::ImageLayout initial_layout);
         ImageBuilder& change_allocation_flags(VmaAllocationCreateFlags allocation_flags);
         ImageBuilder& change_allocation_usage(VmaMemoryUsage allocation_usage);
         ImageBuilder& change_allocation_required_flags(vk::MemoryPropertyFlags required_flags);
         ImageBuilder& change_allocation_preferred_flags(vk::MemoryPropertyFlags preferred_flags);
         ImageBuilder& change_allocation_memory_type_bits(std::uint32_t memory_type_bits);
         ImageBuilder& change_allocation_pool(VmaPool pool);
         ImageBuilder& change_allocation_user_data(void* user_data);
         ImageBuilder& change_allocation_priority(float priority);

         [[nodiscard]] Image build(Device const& device) const;

      private:
         vk::ImageCreateInfo create_info_{};
         VmaAllocationCreateInfo allocation_info_{};
   };
}

#endif