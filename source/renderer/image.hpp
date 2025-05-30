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
         OwnedImage(VmaAllocator const allocator, vk::Image const image, VmaAllocation const memory)
            : allocator{ allocator }
            , image{ image }
            , memory{ memory }
         {
         }

         OwnedImage(OwnedImage const&) = delete;

         OwnedImage(OwnedImage&& other) noexcept
            : allocator{ other.allocator }
            , image{ other.image }
            , memory{ other.memory }
         {
            other.allocator = nullptr;
            other.image = nullptr;
            other.memory = nullptr;
         }

         ~OwnedImage()
         {
            if (allocator)
               vmaDestroyImage(allocator, static_cast<VkImage>(image), memory);
         }

         OwnedImage& operator=(OwnedImage const&) = delete;

         OwnedImage& operator=(OwnedImage&& other) noexcept
         {
            if (this == &other)
               return *this;

            allocator = other.allocator;
            image = other.image;
            memory = other.memory;

            other.allocator = nullptr;
            other.image = nullptr;
            other.memory = nullptr;

            return *this;
         }

         VmaAllocator allocator{};
         vk::Image image{};
         VmaAllocation memory{};
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