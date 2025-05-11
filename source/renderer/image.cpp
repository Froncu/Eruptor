#include "image.hpp"

namespace eru
{
   Image::Image(std::variant<OwnedImage, vk::Image> image, vk::raii::ImageView identical_view,
      vk::Format const format, vk::ImageLayout const layout, vk::Extent3D const extent)
      : image_{ std::move(image) }
      , identical_view_{ std::move(identical_view) }
      , format_{ format }
      , layout_{ layout }
      , extent_{ extent }
   {
   }

   vk::Image Image::image() const
   {
      if (std::holds_alternative<OwnedImage>(image_))
         return std::get<OwnedImage>(image_).image;

      return std::get<vk::Image>(image_);
   }

   vk::raii::ImageView const& Image::identical_view() const
   {
      return identical_view_;
   }

   vk::Format Image::format() const
   {
      return format_;
   }

   vk::ImageLayout Image::layout() const
   {
      return layout_;
   }

   vk::Extent3D Image::extent() const
   {
      return extent_;
   }
}