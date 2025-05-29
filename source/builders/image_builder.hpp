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
         ImageBuilder(ImageBuilder&&) = delete;

         ~ImageBuilder() = default;

         ImageBuilder& operator=(ImageBuilder const&) = delete;
         ImageBuilder& operator=(ImageBuilder&&) = delete;

         ImageBuilder& change_create_info(vk::ImageCreateInfo const& create_info);
         ImageBuilder& change_allocation_info(VmaAllocationCreateInfo const& allocation_info);

         [[nodiscard]] Image build(Device const& device) const;

      private:
         vk::ImageCreateInfo create_info_{};
         VmaAllocationCreateInfo allocation_info_{};
   };
}

#endif