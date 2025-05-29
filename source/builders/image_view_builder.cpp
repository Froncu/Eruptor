#include "image_view_builder.hpp"

namespace eru
{
   ImageViewBuilder& ImageViewBuilder::change_view_type(vk::ImageViewType const view_type)
   {
      view_type_ = view_type;
      return *this;
   }

   ImageViewBuilder& ImageViewBuilder::change_format(vk::Format const format)
   {
      format_ = format;
      return *this;
   }

   ImageViewBuilder& ImageViewBuilder::change_component_mapping(vk::ComponentMapping const component_mapping)
   {
      component_mapping_ = component_mapping;
      return *this;
   }

   ImageViewBuilder& ImageViewBuilder::change_subresource_range(vk::ImageSubresourceRange const& subresource_range)
   {
      subresource_range_ = subresource_range;
      return *this;
   }

   ImageView ImageViewBuilder::build(Device const& device, Image const& image) const
   {
      return ImageView{
         device.device().createImageView({
            .image{ image.image() },
            .viewType{ view_type_ },
            .format{ format_ },
            .components{ component_mapping_ },
            .subresourceRange{ subresource_range_ }
         })
      };
   }
}