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

      private:
         Image(std::variant<vk::raii::Image, vk::Image> image, vk::raii::ImageView view, vk::ImageLayout layout);

         std::variant<vk::raii::Image, vk::Image> image_;
         vk::raii::ImageView view_;
         vk::ImageLayout layout_;
   };
}

#endif