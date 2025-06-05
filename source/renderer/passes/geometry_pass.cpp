#include "geometry_pass.hpp"
#include "builders/pipeline_builder.hpp"
#include "scene/vertex.hpp"

namespace eru
{
   GeometryPass::GeometryPass(Device const& device, vk::Extent2D const swap_chain_extent, DescriptorSets const& descriptor_sets,
      std::uint32_t const frames_in_flight)
      : swap_chain_extent_{ swap_chain_extent }
      , descriptor_sets_{ descriptor_sets }
      , frames_in_flight_{ frames_in_flight }
      , vertex_shader_{ "resources/shaders/geometry.vert", device }
      , fragment_shader_{ "resources/shaders/geometry.frag", device }
      , pipeline_{
         PipelineBuilder{}
         .add_color_attachment_formats(FORMATS)
         .change_depth_attachment_format(vk::Format::eD32Sfloat)
         .add_vertex_bindings(Vertex::BINDING_DESCRIPTIONS)
         .add_vertex_attributes(Vertex::ATTRIBUTE_DESCRIPTIONS)
         .add_shader_stages({
            {
               .stage{ vk::ShaderStageFlagBits::eVertex },
               .module{ *vertex_shader_.module() },
               .pName{ "main" }
            },
            {
               .stage{ vk::ShaderStageFlagBits::eFragment },
               .module{ *fragment_shader_.module() },
               .pName{ "main" }
            }
         })
         .assign_descriptor_set_layout("camera", 0)
         .assign_descriptor_set_layout("texturing", 1)
         .change_depth_stencil_state({
            .depthTestEnable{ true },
            .depthCompareOp{ vk::CompareOp::eEqual }
         })
         .add_color_blend_attachment_state({
            .blendEnable{ true },
            .srcColorBlendFactor{ vk::BlendFactor::eSrcAlpha },
            .dstColorBlendFactor{ vk::BlendFactor::eOneMinusSrcAlpha },
            .colorBlendOp{ vk::BlendOp::eAdd },
            .srcAlphaBlendFactor{ vk::BlendFactor::eOne },
            .dstAlphaBlendFactor{ vk::BlendFactor::eZero },
            .alphaBlendOp{ vk::BlendOp::eAdd },
            .colorWriteMask{
               vk::ColorComponentFlagBits::eR |
               vk::ColorComponentFlagBits::eG |
               vk::ColorComponentFlagBits::eB |
               vk::ColorComponentFlagBits::eA
            }
         }, frames_in_flight_)
         .add_push_constant_range({
            .stageFlags{ vk::ShaderStageFlagBits::eFragment },
            .offset{ 0 },
            .size{ sizeof(std::uint32_t) }
         })
         .build(device, descriptor_sets_)
      }
      , image_builder_{
         ImageBuilder{}
         .change_type(vk::ImageType::e2D)
         .change_extent(swap_chain_extent_)
         .change_mip_levels(1)
         .change_array_layers(1)
         .change_samples(vk::SampleCountFlagBits::e1)
         .change_tiling(vk::ImageTiling::eOptimal)
         .change_usage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled)
         .change_sharing_mode(vk::SharingMode::eExclusive)
         .change_initial_layout(vk::ImageLayout::eUndefined)
         .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
      }
      , geometry_images_{ create_geometry_images(device) }
      , image_view_builder_{
         ImageViewBuilder{}
         .change_view_type(vk::ImageViewType::e2D)
         .change_subresource_range({
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         })
      }
      , geometry_image_views_{ create_geometry_image_views(device) }
   {
      write_descriptor_sets(device);
   }

   std::span<ImageView const> GeometryPass::position_image_view() const
   {
      return geometry_image_views_[0];
   }

   std::span<ImageView const> GeometryPass::base_color_image_view() const
   {
      return geometry_image_views_[1];
   }

   std::span<ImageView const> GeometryPass::normal_image_view() const
   {
      return geometry_image_views_[2];
   }

   std::span<ImageView const> GeometryPass::metalness_image_view() const
   {
      return geometry_image_views_[3];
   }

   void GeometryPass::render(vk::raii::CommandBuffer const& command_buffer, Scene const& scene,
      std::uint32_t const current_frame, ImageView const& depth_image_view) const
   {
      std::array<vk::ImageMemoryBarrier2, FORMATS.size()> begin_barriers{};
      for (std::size_t index{}; index < FORMATS.size(); ++index)
      {
         begin_barriers[index].srcStageMask = vk::PipelineStageFlagBits2::eNone;
         begin_barriers[index].srcAccessMask = vk::AccessFlagBits2::eNone;
         begin_barriers[index].dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
         begin_barriers[index].dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
         begin_barriers[index].oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
         begin_barriers[index].newLayout = vk::ImageLayout::eColorAttachmentOptimal;
         begin_barriers[index].image = geometry_images_[index][current_frame].image();
         begin_barriers[index].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
         begin_barriers[index].subresourceRange.levelCount = 1;
         begin_barriers[index].subresourceRange.layerCount = 1;
      }

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ static_cast<std::uint32_t>(begin_barriers.size()) },
         .pImageMemoryBarriers{ begin_barriers.data() }
      });

      std::array<vk::RenderingAttachmentInfo, FORMATS.size()> color_attachment_infos{};
      for (std::size_t index{}; index < FORMATS.size(); ++index)
      {
         color_attachment_infos[index].imageView = geometry_image_views_[index][current_frame].image_view();
         color_attachment_infos[index].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
         color_attachment_infos[index].loadOp = vk::AttachmentLoadOp::eClear;
         color_attachment_infos[index].storeOp = vk::AttachmentStoreOp::eStore;
         color_attachment_infos[index].clearValue.color = std::array{ 0.0f, 0.0f, 0.0f, 1.0f };
      };

      vk::RenderingAttachmentInfo const depth_attachment_info{
         .imageView{ *depth_image_view.image_view() },
         .imageLayout{ vk::ImageLayout::eDepthStencilAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eLoad },
         .storeOp{ vk::AttachmentStoreOp::eStore },
      };

      command_buffer.beginRendering({
         .renderArea{
            .extent{ swap_chain_extent_ }
         },
         .layerCount{ 1 },
         .colorAttachmentCount{ static_cast<std::uint32_t>(color_attachment_infos.size()) },
         .pColorAttachments{ color_attachment_infos.data() },
         .pDepthAttachment{ &depth_attachment_info }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.pipeline());

      command_buffer.setViewport(0, {
         {
            .width{ static_cast<float>(swap_chain_extent_.width) },
            .height{ static_cast<float>(swap_chain_extent_.height) },
            .maxDepth{ 1.0f }
         }
      });

      command_buffer.setScissor(0, {
         {
            .extent{ swap_chain_extent_ }
         }
      });

      command_buffer.bindVertexBuffers(0, { scene.vertex_buffer().buffer() }, { {} });
      command_buffer.bindIndexBuffer(scene.index_buffer().buffer(), 0, vk::IndexType::eUint32);
      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.layout(), 0,
         {
            descriptor_sets_.sets("camera")[current_frame],
            descriptor_sets_.sets("texturing").front()
         }, {});

      for (auto const& [vertex_offset, index_offset, index_count, material_index] : scene.sub_meshes())
      {
         command_buffer.pushConstants<std::uint32_t>(
            *pipeline_.layout(),
            vk::ShaderStageFlagBits::eFragment,
            0,
            material_index);

         command_buffer.drawIndexed(
            index_count,
            1,
            index_offset,
            vertex_offset,
            0);
      }

      command_buffer.endRendering();

      std::array<vk::ImageMemoryBarrier2, FORMATS.size()> end_barriers{};
      for (std::size_t index{}; index < FORMATS.size(); ++index)
      {
         end_barriers[index].srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
         end_barriers[index].srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
         end_barriers[index].dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
         end_barriers[index].dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead;
         end_barriers[index].oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
         end_barriers[index].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
         end_barriers[index].image = geometry_images_[index][current_frame].image();
         end_barriers[index].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
         end_barriers[index].subresourceRange.levelCount = 1;
         end_barriers[index].subresourceRange.layerCount = 1;
      }

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ static_cast<std::uint32_t>(end_barriers.size()) },
         .pImageMemoryBarriers{ end_barriers.data() }
      });
   }

   void GeometryPass::recreate_geometry_images(Device const& device, vk::Extent2D const swap_chain_extent)
   {
      image_builder_.change_extent(swap_chain_extent);
      geometry_images_ = create_geometry_images(device);
      geometry_image_views_ = create_geometry_image_views(device);
      write_descriptor_sets(device);
      swap_chain_extent_ = swap_chain_extent;
   }

   std::array<std::vector<Image>, GeometryPass::FORMATS.size()> GeometryPass::create_geometry_images(Device const& device)
   {
      std::array<std::vector<Image>, FORMATS.size()> geometry_images{};
      for (std::size_t index{}; index < FORMATS.size(); ++index)
      {
         vk::Format const format{ FORMATS[index] };
         std::vector<Image>& images{ geometry_images[index] };

         images.reserve(frames_in_flight_);
         for (std::uint32_t frame_index{}; frame_index < frames_in_flight_; ++frame_index)
            images.emplace_back(image_builder_.change_format(format).build(device))
                  .transition_layout(device, vk::ImageLayout::eShaderReadOnlyOptimal);
      }

      return geometry_images;
   }

   std::array<std::vector<ImageView>, GeometryPass::FORMATS.size()> GeometryPass::create_geometry_image_views(
      Device const& device)
   {
      std::array<std::vector<ImageView>, FORMATS.size()> geometry_image_views{};
      for (std::size_t index{}; index < FORMATS.size(); ++index)
      {
         std::vector<ImageView>& views{ geometry_image_views[index] };
         views.reserve(geometry_images_[index].size());

         for (Image const& image : geometry_images_[index])
            views.emplace_back(image_view_builder_.change_format(image.info().format).build(device, image));
      }

      return geometry_image_views;
   }

   void GeometryPass::write_descriptor_sets(Device const& device)
   {
      std::vector<vk::DescriptorImageInfo> image_infos{};
      image_infos.reserve(FORMATS.size() * frames_in_flight_);
      std::vector<vk::WriteDescriptorSet> writes{};
      writes.reserve(FORMATS.size() * frames_in_flight_);

      for (std::uint32_t format_index{}; format_index < FORMATS.size(); ++format_index)
         for (std::uint32_t image_index{}; image_index < frames_in_flight_; ++image_index)
         {
            image_infos.push_back({
               .imageView{ *geometry_image_views_[format_index][image_index].image_view() },
               .imageLayout{ vk::ImageLayout::eShaderReadOnlyOptimal }
            });

            writes.push_back({
               .dstSet{ *descriptor_sets_.sets("geometry").front() },
               .dstBinding{ descriptor_sets_.binding("geometry", "position") + format_index },
               .dstArrayElement{ image_index },
               .descriptorCount{ 1 },
               .descriptorType{ vk::DescriptorType::eSampledImage },
               .pImageInfo{ &image_infos.back() }
            });
         }

      device.device().updateDescriptorSets(writes, {});
   }
}