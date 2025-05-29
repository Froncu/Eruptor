#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   class Image
   {
      friend class ImageBuilder;
      friend class SwapChainBuilder;

      struct OwnedImage final
      {
         ~OwnedImage()
         {
            vmaDestroyImage(allocator, static_cast<VkImage>(image), memory);
         }

         VmaAllocator allocator;
         vk::Image image;
         VmaAllocation memory;
      };

      public:
         Image(Image const&) = delete;
         Image(Image&&) = default;

         ~Image() = default;

         Image& operator=(Image const&) = delete;
         Image& operator=(Image&&) = default;

         [[nodiscard]] vk::Image image() const;
         [[nodiscard]] vk::ImageCreateInfo const& info() const;

      private:
         Image(std::variant<OwnedImage, vk::Image> image, vk::ImageCreateInfo info);

         std::variant<OwnedImage, vk::Image> image_;
         vk::ImageCreateInfo info_;
   };
}

#endif