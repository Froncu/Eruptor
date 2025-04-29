#include "image.hpp"

namespace eru
{
   Image::Image(std::variant<vk::raii::Image, vk::Image> image, vk::raii::ImageView view, vk::ImageLayout const layout)
      : image_{ std::move(image) }
      , view_{ std::move(view) }
      , layout_{ layout }
   {
   }
}