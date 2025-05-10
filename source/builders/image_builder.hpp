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
         ImageBuilder(ImageBuilder const&) = delete;
         ImageBuilder(ImageBuilder&&) = default;

         ~ImageBuilder() = default;

         ImageBuilder& operator=(ImageBuilder const&) = delete;
         ImageBuilder& operator=(ImageBuilder&&) = default;

         ImageBuilder& change_create_info(vk::ImageCreateInfo const& create_info, vk::ImageAspectFlags identical_aspect_flags);
         ImageBuilder& change_allocation_info(VmaAllocationCreateInfo const& allocation_info);

         [[nodiscard]] Image build(Device const& device) const;

      private:
         vk::ImageCreateInfo create_info_{};
         vk::ImageAspectFlags identical_aspect_flags_{};
         VmaAllocationCreateInfo allocation_info_{};
   };
}

#endif