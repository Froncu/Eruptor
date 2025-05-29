#ifndef IMAGE_VIEW_HPP
#define IMAGE_VIEW_HPP

namespace eru
{
   class ImageView final
   {
      friend class ImageViewBuilder;

      public:
         ImageView(ImageView const&) = delete;
         ImageView(ImageView&&) = default;

         ~ImageView() = default;

         ImageView& operator=(ImageView const&) = delete;
         ImageView& operator=(ImageView&&) = default;

         [[nodiscard]] vk::raii::ImageView const& image_view() const;

      private:
         explicit ImageView(vk::raii::ImageView image_view);

         vk::raii::ImageView image_view_;
   };
}

#endif