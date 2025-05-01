#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   class Image
   {
      friend class SwapChainBuilder;

      public:
         Image(Image const&) = delete;
         Image(Image&&) = default;

         ~Image() = default;

         Image& operator=(Image const&) = delete;
         Image& operator=(Image&&) = default;

         [[nodiscard]] vk::Image image() const;
         [[nodiscard]] vk::raii::ImageView const& view() const;
         [[nodiscard]] vk::Format format() const;
         [[nodiscard]] vk::ImageLayout layout() const;

      private:
         Image(std::variant<vk::raii::Image, vk::Image> image, vk::raii::ImageView view, vk::Format format, vk::ImageLayout layout);

         std::variant<vk::raii::Image, vk::Image> image_;
         vk::raii::ImageView view_;
         vk::Format format_;
         vk::ImageLayout layout_;
   };
}

#endif