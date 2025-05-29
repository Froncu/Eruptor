#include "image.hpp"

namespace eru
{
   Image::Image(std::variant<OwnedImage, vk::Image> image, vk::ImageCreateInfo info)
      : image_{ std::move(image) }
      , info_{ std::move(info) }
   {
   }

   vk::Image Image::image() const
   {
      if (std::holds_alternative<OwnedImage>(image_))
         return std::get<OwnedImage>(image_).image;

      return std::get<vk::Image>(image_);
   }

   vk::ImageCreateInfo const& Image::info() const
   {
      return info_;
   }
}