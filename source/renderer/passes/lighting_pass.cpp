#include "lighting_pass.hpp"
#include "builders/graphics_pipeline_builder.hpp"

namespace eru
{
   LightingPass::LightingPass(Device const& device, vk::Extent2D const swap_chain_extent, DescriptorSets const& descriptor_sets,
      std::uint32_t const frames_in_flight)
      : swap_chain_extent_{ swap_chain_extent }
      , descriptor_sets_{ descriptor_sets }
      , frames_in_flight_{ frames_in_flight }
      , vertex_shader_{ "resources/shaders/fullscreen_quad.vert", device }
      , fragment_shader_{ "resources/shaders/lighting.frag", device }
      , pipeline_{
         GraphicsPipelineBuilder{}
         .add_color_attachment_format(vk::Format::eR32G32B32A32Sfloat)
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
         .change_input_assembly_state({
            .topology{ vk::PrimitiveTopology::eTriangleStrip }
         })
         .assign_descriptor_set_layout("geometry", 0)
         .change_depth_stencil_state({})
         .add_push_constant_range({
            .stageFlags{ vk::ShaderStageFlagBits::eFragment },
            .offset{ 0 },
            .size{ sizeof(PushConstants) }
         })
         .build(device, descriptor_sets)
      }
      , image_builder_{
         ImageBuilder{}
         .change_type(vk::ImageType::e2D)
         .change_format(vk::Format::eR32G32B32A32Sfloat)
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
      , images_{ create_images(device) }
      , image_views_{ create_image_views(device) }
   {
      write_descriptor_sets(device);
   }

   void LightingPass::render(vk::raii::CommandBuffer const& command_buffer, glm::vec3 const camera_position,
      std::uint32_t const current_frame) const
   {
      vk::ImageMemoryBarrier2 const begin_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .oldLayout{ vk::ImageLayout::eShaderReadOnlyOptimal },
         .newLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .image{ images_[current_frame].image() },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &begin_barrier }
      });

      vk::RenderingAttachmentInfo const color_attachment_info{
         .imageView{ *image_views_[current_frame].image_view() },
         .imageLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .clearValue{
            vk::ClearColorValue{
               std::array{ 0.0f, 0.0f, 0.0f, 1.0f }
            }
         }
      };

      command_buffer.beginRendering({
         .renderArea{
            .extent{ swap_chain_extent_ }
         },
         .layerCount{ 1 },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &color_attachment_info }
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

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.layout(), 0,
         {
            descriptor_sets_.sets("geometry").front()
         }, {});

      command_buffer.pushConstants<PushConstants>(*pipeline_.layout(), vk::ShaderStageFlagBits::eFragment, 0, {
         {
            .camera_position{ camera_position },
            .current_frame{ current_frame }
         }
      });

      command_buffer.draw(4, 1, 0, 0);

      command_buffer.endRendering();

      vk::ImageMemoryBarrier2 const end_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .srcAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eFragmentShader },
         .dstAccessMask{ vk::AccessFlagBits2::eShaderSampledRead },
         .oldLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .newLayout{ vk::ImageLayout::eShaderReadOnlyOptimal },
         .image{ images_[current_frame].image() },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &end_barrier }
      });
   }

   void LightingPass::recreate_hdr_images(Device const& device, vk::Extent2D const swap_chain_extent)
   {
      image_builder_.change_extent(swap_chain_extent);
      images_ = create_images(device);
      image_views_ = create_image_views(device);
      write_descriptor_sets(device);
      swap_chain_extent_ = swap_chain_extent;
   }

   std::vector<Image> LightingPass::create_images(Device const& device) const
   {
      std::vector<Image> images{};
      images.reserve(frames_in_flight_);
      for (std::uint32_t index{}; index < frames_in_flight_; ++index)
         images.emplace_back(image_builder_.build(device))
               .transition_layout(device, vk::ImageLayout::eShaderReadOnlyOptimal);

      return images;
   }

   std::vector<ImageView> LightingPass::create_image_views(Device const& device) const
   {
      std::vector<ImageView> views{};
      views.reserve(images_.size());
      for (auto const& image : images_)
         views.emplace_back(image_view_builder_.build(device, image));

      return views;
   }

   void LightingPass::write_descriptor_sets(Device const& device) const
   {
      std::vector<vk::DescriptorImageInfo> image_infos{};
      image_infos.reserve(frames_in_flight_);
      std::vector<vk::WriteDescriptorSet> writes{};
      writes.reserve(frames_in_flight_);

      for (std::uint32_t index{}; index < frames_in_flight_; ++index)
      {
         image_infos.push_back({
            .imageView{ *image_views_[index].image_view() },
            .imageLayout{ vk::ImageLayout::eShaderReadOnlyOptimal }
         });

         writes.push_back({
            .dstSet{ *descriptor_sets_.sets("hdr").front() },
            .dstBinding{ descriptor_sets_.binding("hdr", "image") },
            .dstArrayElement{ index },
            .descriptorCount{ 1 },
            .descriptorType{ vk::DescriptorType::eSampledImage },
            .pImageInfo{ &image_infos.back() }
         });
      }

      device.device().updateDescriptorSets(writes, {});
   }
}