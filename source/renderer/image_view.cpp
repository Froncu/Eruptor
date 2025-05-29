#include "image_view.hpp"

namespace eru
{
   ImageView::ImageView(vk::raii::ImageView image_view)
      : image_view_{ std::move(image_view) }
   {
   }

   vk::raii::ImageView const& ImageView::image_view() const
   {
      return image_view_;
   }
}