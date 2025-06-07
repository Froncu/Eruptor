#ifndef LIGHTING_PASS_HPP
#define LIGHTING_PASS_HPP

#include "builders/image_builder.hpp"
#include "builders/image_view_builder.hpp"
#include "erupch/erupch.hpp"
#include "renderer/descriptor_sets.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/shader.hpp"

namespace eru
{
   class LightingPass final
   {
      public:
         LightingPass(Device const& device, vk::Extent2D swap_chain_extent, DescriptorSets const& descriptor_sets,
            std::uint32_t frames_in_flight);
         LightingPass(LightingPass const&) = delete;
         LightingPass(LightingPass&&) = default;

         ~LightingPass() = default;

         LightingPass& operator=(LightingPass const&) = delete;
         LightingPass& operator=(LightingPass&&) = delete;

         void render(vk::raii::CommandBuffer const& command_buffer, std::uint32_t current_frame,
            glm::vec3 camera_position) const;
         void recreate_hdr_images(Device const& device, vk::Extent2D swap_chain_extent);

      private:
         struct alignas(16) PushConstants
         {
            glm::vec3 camera_position;
            std::uint32_t current_frame;
         };

         std::vector<Image> create_images(Device const& device) const;
         std::vector<ImageView> create_image_views(Device const& device) const;
         void write_descriptor_sets(Device const& device) const;

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
               .aspectMask{ vk::ImageAspectFlagBits::eColor },
               .levelCount{ 1 },
               .layerCount{ 1 }
            })
         };
         std::vector<ImageView> image_views_;
   };
}

#endif