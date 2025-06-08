#ifndef DEPTH_PASS_HPP
#define DEPTH_PASS_HPP

#include "builders/image_builder.hpp"
#include "builders/image_view_builder.hpp"
#include "renderer/descriptor_sets.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/shader.hpp"
#include "scene/scene.hpp"

namespace eru
{
   class DepthPass final
   {
      public:
         DepthPass(Device const& device, vk::Extent2D swap_chain_extent, DescriptorSets const& descriptor_sets,
            std::uint32_t frames_in_flight);
         DepthPass(DepthPass const&) = delete;
         DepthPass(DepthPass&&) = default;

         ~DepthPass() = default;

         DepthPass& operator=(DepthPass const&) = delete;
         DepthPass& operator=(DepthPass&&) = delete;

         [[nodiscard]] std::span<ImageView const> depth_image_views() const;

         void render(vk::raii::CommandBuffer const& command_buffer, Scene const& scene, std::uint32_t current_frame) const;
         void recreate_depth_images(Device const& device, vk::Extent2D swap_chain_extent);

      private:
         std::vector<Image> create_images(Device const& device) const;
         std::vector<ImageView> create_image_views(Device const& device) const;

         vk::Extent2D swap_chain_extent_;
         DescriptorSets const& descriptor_sets_;
         std::uint32_t const frames_in_flight_;

         Shader vertex_shader_;
         Shader fragment_shader_;
         Pipeline pipeline_;

         ImageBuilder image_builder_;
         std::vector<Image> images_;
         ImageViewBuilder image_view_builder_{
            ImageViewBuilder{}
            .change_view_type(vk::ImageViewType::e2D)
            .change_format(images_.front().info().format)
            .change_subresource_range({
               .aspectMask{ vk::ImageAspectFlagBits::eDepth },
               .levelCount{ 1 },
               .layerCount{ 1 }
            })
         };
         std::vector<ImageView> image_views_;
   };
}

#endif