#ifndef GEOMETRY_PASS_HPP
#define GEOMETRY_PASS_HPP

#include "builders/image_builder.hpp"
#include "builders/image_view_builder.hpp"
#include "renderer/descriptor_sets.hpp"
#include "renderer/device.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/shader.hpp"
#include "scene/scene.hpp"

namespace eru
{
   class GeometryPass final
   {
      static std::array<vk::Format, 4> constexpr FORMATS{
         vk::Format::eR32G32B32A32Sfloat, // Position
         vk::Format::eR8G8B8A8Srgb,       // Base Color
         vk::Format::eR8G8B8A8Unorm,      // Normal
         vk::Format::eR8G8B8A8Unorm       // Metalness
      };

      public:
         GeometryPass(Device const& device, vk::Extent2D swap_chain_extent, DescriptorSets const& descriptor_sets,
            std::uint32_t frames_in_flight);
         GeometryPass(GeometryPass const&) = delete;
         GeometryPass(GeometryPass&&) = default;

         ~GeometryPass() = default;

         GeometryPass& operator=(GeometryPass const&) = delete;
         GeometryPass& operator=(GeometryPass&&) = delete;

         [[nodiscard]] std::span<ImageView const> position_image_view() const;
         [[nodiscard]] std::span<ImageView const> base_color_image_view() const;
         [[nodiscard]] std::span<ImageView const> normal_image_view() const;
         [[nodiscard]] std::span<ImageView const> metalness_image_view() const;

         void render(vk::raii::CommandBuffer const& command_buffer, Scene const& scene, std::uint32_t current_frame,
            ImageView const& depth_image_view) const;
         void recreate_geometry_images(Device const& device, vk::Extent2D swap_chain_extent);

      private:
         [[nodiscard]] std::array<std::vector<Image>, FORMATS.size()> create_geometry_images(Device const& device);
         [[nodiscard]] std::array<std::vector<ImageView>, FORMATS.size()> create_geometry_image_views(Device const& device);
         void write_descriptor_sets(Device const& device) const;
 
         vk::Extent2D swap_chain_extent_;
         DescriptorSets const& descriptor_sets_;
         std::uint32_t const frames_in_flight_;

         Shader vertex_shader_;
         Shader fragment_shader_;
         Pipeline pipeline_;

         ImageBuilder image_builder_;
         std::array<std::vector<Image>, FORMATS.size()> geometry_images_;
         ImageViewBuilder image_view_builder_{
            ImageViewBuilder{}
            .change_view_type(vk::ImageViewType::e2D)
            .change_subresource_range({
               .aspectMask{ vk::ImageAspectFlagBits::eDepth },
               .levelCount{ 1 },
               .layerCount{ 1 }
            })
         };
         std::array<std::vector<ImageView>, FORMATS.size()> geometry_image_views_;
   };
}

#endif