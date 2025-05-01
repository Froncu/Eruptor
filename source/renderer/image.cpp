#include "image.hpp"

namespace eru
{
   Image::Image(std::variant<vk::raii::Image, vk::Image> image, vk::raii::ImageView view,
      vk::Format const format, vk::ImageLayout const layout)
      : image_{ std::move(image) }
      , view_{ std::move(view) }
      , format_{ format }
      , layout_{ layout }
   {
   }

   vk::Image Image::image() const
   {
      if (std::holds_alternative<vk::raii::Image>(image_))
         return *std::get<vk::raii::Image>(image_);

      return std::get<vk::Image>(image_);
   }

   vk::raii::ImageView const& Image::view() const
   {
      return view_;
   }

   vk::Format Image::format() const
   {
      return format_;
   }

   vk::ImageLayout Image::layout() const
   {
      return layout_;
   }
}