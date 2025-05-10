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
         vk::raii::Image image;
         VmaAllocation memory;
      };

      public:
         Image(Image const&) = delete;
         Image(Image&&) = default;

         ~Image() = default;

         Image& operator=(Image const&) = delete;
         Image& operator=(Image&&) = default;

         [[nodiscard]] vk::Image image() const;
         [[nodiscard]] vk::raii::ImageView const& identical_view() const;
         [[nodiscard]] vk::Format format() const;
         [[nodiscard]] vk::ImageLayout layout() const;

      private:
         Image(std::variant<OwnedImage, vk::Image> image, vk::raii::ImageView identical_view, vk::Format format,
            vk::ImageLayout layout);

         std::variant<OwnedImage, vk::Image> image_;
         vk::raii::ImageView identical_view_;
         vk::Format format_;
         vk::ImageLayout layout_;
   };
}

#endif