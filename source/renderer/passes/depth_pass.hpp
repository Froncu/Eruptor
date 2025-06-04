#ifndef DEPTH_PASS_HPP
#define DEPTH_PASS_HPP

#include "builders/image_builder.hpp"
#include "builders/image_view_builder.hpp"
#include "builders/pipeline_builder.hpp"
#include "renderer/shader.hpp"
#include "scene/scene.hpp"

namespace eru
{
   class DepthPass final
   {
      public:
         DepthPass(Device const& device, vk::Extent2D swap_chain_extent, DescriptorSets const& descriptor_sets);
         DepthPass(DepthPass const&) = delete;
         DepthPass(DepthPass&&) = default;

         ~DepthPass() = default;

         DepthPass& operator=(DepthPass const&) = delete;
         DepthPass& operator=(DepthPass&&) = delete;

         [[nodiscard]] ImageView const& depth_image_view() const;

         void render(vk::raii::CommandBuffer const& command_buffer, Scene const& scene, std::uint32_t current_frame) const;
         void recreate_depth_image(Device const& device, vk::Extent2D swap_chain_extent);

      private:
         vk::Extent2D swap_chain_extent_;
         DescriptorSets const& descriptor_sets_;
         Shader vertex_shader_;
         Pipeline pipeline_;
         ImageBuilder depth_image_builder_;
         Image depth_image_;
         ImageViewBuilder depth_image_view_builder_{
            ImageViewBuilder{}
            .change_view_type(vk::ImageViewType::e2D)
            .change_format(depth_image_.info().format)
            .change_subresource_range({
               .aspectMask{ vk::ImageAspectFlagBits::eDepth },
               .levelCount{ 1 },
               .layerCount{ 1 }
            })
         };
         ImageView depth_image_view_;
   };
}

#endif