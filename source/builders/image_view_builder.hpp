#ifndef IMAGE_VIEW_BUILDER_HPP
#define IMAGE_VIEW_BUILDER_HPP

#include "renderer/device.hpp"
#include "renderer/image.hpp"
#include "renderer/image_view.hpp"

namespace eru
{
   class ImageViewBuilder final
   {
      public:
         ImageViewBuilder() = default;
         ImageViewBuilder(ImageViewBuilder const&) = default;
         ImageViewBuilder(ImageViewBuilder&&) = default;

         ~ImageViewBuilder() = default;

         ImageViewBuilder& operator=(ImageViewBuilder const&) = default;
         ImageViewBuilder& operator=(ImageViewBuilder&&) = default;

         ImageViewBuilder& change_view_type(vk::ImageViewType view_type);
         ImageViewBuilder& change_format(vk::Format format);
         ImageViewBuilder& change_component_mapping(vk::ComponentMapping component_mapping);
         ImageViewBuilder& change_subresource_range(vk::ImageSubresourceRange const& subresource_range);

         [[nodiscard]] ImageView build(Device const& device, Image const& image) const;

      private:
         vk::ImageViewType view_type_{ vk::ImageViewType::e2D };
         vk::Format format_{ vk::Format::eUndefined };
         vk::ComponentMapping component_mapping_{
            .r{ vk::ComponentSwizzle::eIdentity },
            .g{ vk::ComponentSwizzle::eIdentity },
            .b{ vk::ComponentSwizzle::eIdentity },
            .a{ vk::ComponentSwizzle::eIdentity }
         };
         vk::ImageSubresourceRange subresource_range_{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .baseMipLevel{ 0 },
            .levelCount{ 1 },
            .baseArrayLayer{ 0 },
            .layerCount{ 1 }
         };
   };
}

#endif