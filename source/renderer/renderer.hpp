#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "buffer.hpp"
#include "camera.hpp"
#include "device.hpp"
#include "builders/buffer_builder.hpp"
#include "builders/descriptor_sets_builder.hpp"
#include "builders/device_builder.hpp"
#include "builders/graphics_pipeline_builder.hpp"
#include "builders/image_builder.hpp"
#include "builders/swap_chain_builder.hpp"
#include "passes/depth_pass.hpp"
#include "passes/geometry_pass.hpp"
#include "passes/lighting_pass.hpp"
#include "passes/tone_map_pass.hpp"
#include "scene/scene.hpp"
#include "window/window.hpp"

namespace eru
{
   class Renderer final
   {
      public:
         explicit Renderer(Window const& window);
         Renderer(Renderer const&) = delete;
         Renderer(Renderer&&) = delete;

         ~Renderer();

         Renderer& operator=(Renderer const&) = delete;
         Renderer& operator=(Renderer&&) = delete;

         void render();

         Camera camera{};

      private:
         static std::uint32_t constexpr FRAMES_IN_FLIGHT{ 3 };

         Reference<Window const> const window_;

         Device device_{
            DeviceBuilder{}
            .enable_extension(vk::KHRSwapchainExtensionName)
            .enable_features10({ .samplerAnisotropy{ true } })
            .enable_features12({
               .shaderSampledImageArrayNonUniformIndexing{ true },
               .descriptorBindingPartiallyBound{ true }, // QUESTION: my bindless setup worked without this, why?
               .runtimeDescriptorArray{ true }
            })
            .enable_features13({
               .synchronization2{ true },
               .dynamicRendering{ true }
            })
            .add_queues({
               vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eCompute,
               window_->surface()
            })
            .build()
         };
         DescriptorSets descriptor_sets_{
            DescriptorSetsBuilder{}
            .add_descriptor_sets({
               {
                  .name{ "camera" },
                  .bindings{
                     {
                        .name{ "data" },
                        .type{ vk::DescriptorType::eUniformBuffer },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment },
                     }
                  },
                  .allocation_count{ FRAMES_IN_FLIGHT }
               },
               {
                  .name{ "texturing" },
                  .bindings{
                     {
                        .name{ "materials" },
                        .flags{ vk::DescriptorBindingFlagBits::ePartiallyBound },
                        .type{ vk::DescriptorType::eStorageBuffer },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ 100 }
                     },
                     {
                        .name{ "sampler" },
                        .type{ vk::DescriptorType::eSampler },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment }
                     },
                     {
                        .name{ "base_color_textures" },
                        .flags{ vk::DescriptorBindingFlagBits::ePartiallyBound },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ 100 }
                     },
                     {
                        .name{ "normal_textures" },
                        .flags{ vk::DescriptorBindingFlagBits::ePartiallyBound },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ 100 }
                     },
                     {
                        .name{ "metalness_textures" },
                        .flags{ vk::DescriptorBindingFlagBits::ePartiallyBound },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ 100 }
                     }
                  }
               },
               {
                  .name{ "geometry" },
                  .bindings{
                     {
                        .name{ "sampler" },
                        .type{ vk::DescriptorType::eSampler },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment }
                     },
                     {
                        .name{ "position" },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ FRAMES_IN_FLIGHT }
                     },
                     {
                        .name{ "base_color" },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ FRAMES_IN_FLIGHT }
                     },
                     {
                        .name{ "normal" },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ FRAMES_IN_FLIGHT }
                     },
                     {
                        .name{ "metalness" },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment },
                        .count{ FRAMES_IN_FLIGHT }
                     }
                  }
               },
               {
                  .name{ "hdr" },
                  .bindings{
                     {
                        .name{ "sampler" },
                        .type{ vk::DescriptorType::eSampler },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment }
                     },
                     {
                        .name{ "image" },
                        .type{ vk::DescriptorType::eSampledImage },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute },
                        .count{ FRAMES_IN_FLIGHT }
                     }
                  }
               },
               {
                  .name{ "histogram" },
                  .bindings{
                     {
                        .name{ "bins" },
                        .type{ vk::DescriptorType::eStorageBuffer },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eCompute },
                        .count{ 256 }
                     }
                  },
                  .allocation_count{ FRAMES_IN_FLIGHT }
               },
               {
                  .name{ "avarage_luminance" },
                  .bindings{
                     {
                        .name{ "value" },
                        .type{ vk::DescriptorType::eStorageBuffer },
                        .shader_stage_flags{ vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment }
                     }
                  },
                  .allocation_count{ FRAMES_IN_FLIGHT }
               }
            })
            .build(device_)
         };

         SwapChainBuilder swap_chain_builder_{};
         SwapChain swap_chain_{
            swap_chain_builder_
            .change_format({ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
            .change_present_mode(vk::PresentModeKHR::eMailbox)
            .build(device_, *window_, device_.queues())
         };

         DepthPass depth_pass_{ device_, swap_chain_.extent(), descriptor_sets_, FRAMES_IN_FLIGHT };
         GeometryPass geometry_pass_{ device_, swap_chain_.extent(), descriptor_sets_, FRAMES_IN_FLIGHT };
         LightingPass lighting_pass_{ device_, swap_chain_.extent(), descriptor_sets_, FRAMES_IN_FLIGHT };
         ToneMapPass tone_map_pass_{ device_, swap_chain_.images().front().info().format, descriptor_sets_ };

         std::vector<Buffer> camera_buffers_{
            [this]
            {
               vk::DeviceSize constexpr buffer_size{ sizeof(Camera::Data) };
               auto const buffer_builder{
                  BufferBuilder{}
                  .change_size(buffer_size)
                  .change_usage(vk::BufferUsageFlagBits::eUniformBuffer)
                  .change_sharing_mode(vk::SharingMode::eExclusive)
                  .change_allocation_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT)
                  .change_allocation_required_flags(
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
               };

               std::vector<Buffer> buffers{};
               buffers.reserve(FRAMES_IN_FLIGHT);
               std::vector<vk::DescriptorBufferInfo> infos{};
               infos.reserve(FRAMES_IN_FLIGHT);
               std::vector<vk::WriteDescriptorSet> writes{};
               writes.reserve(FRAMES_IN_FLIGHT);
               for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
               {
                  buffers.push_back(buffer_builder.build(device_));

                  infos.push_back({
                     .buffer{ buffers.back().buffer() },
                     .offset{ 0 },
                     .range{ buffer_size }
                  });

                  writes.push_back({
                     .dstSet{ *descriptor_sets_.sets("camera")[index] },
                     .dstBinding{ descriptor_sets_.binding("camera", "data") },
                     .dstArrayElement{ 0 },
                     .descriptorCount{ 1 },
                     .descriptorType{ vk::DescriptorType::eUniformBuffer },
                     .pBufferInfo{ &infos.back() }
                  });
               }

               device_.device().updateDescriptorSets(writes, {});

               return buffers;
            }()
         };

         Scene scene_{ device_, "resources/models/sponza/sponza.gltf" };

         vk::raii::Sampler sampler_{
            [this]
            {
               std::size_t const writes_count{
                  3 +
                  scene_.base_color_images().size() +
                  scene_.normal_images().size() +
                  scene_.metalness_images().size()
               };
               std::vector<vk::DescriptorImageInfo> infos{};
               infos.reserve(writes_count - 1);
               std::vector<vk::WriteDescriptorSet> writes{};
               writes.reserve(writes_count);

               vk::raii::Sampler sampler{
                  device_.device().createSampler({
                     .magFilter{ vk::Filter::eLinear },
                     .minFilter{ vk::Filter::eLinear },
                     .mipmapMode{ vk::SamplerMipmapMode::eLinear },
                     .addressModeU{ vk::SamplerAddressMode::eRepeat },
                     .addressModeV{ vk::SamplerAddressMode::eRepeat },
                     .addressModeW{ vk::SamplerAddressMode::eRepeat },
                     .anisotropyEnable{ true },
                     .maxAnisotropy{ device_.physical_device().getProperties().limits.maxSamplerAnisotropy },
                     .compareOp{ vk::CompareOp::eAlways },
                     .borderColor{ vk::BorderColor::eIntOpaqueBlack },
                  })
               };

               infos.push_back({
                  .sampler{ *sampler }
               });

               writes.push_back({
                  .dstSet{ *descriptor_sets_.sets("texturing").front() },
                  .dstBinding{ descriptor_sets_.binding("texturing", "sampler") },
                  .dstArrayElement{ 0 },
                  .descriptorCount{ 1 },
                  .descriptorType{ vk::DescriptorType::eSampler },
                  .pImageInfo{ &infos.back() }
               });

               writes.push_back({
                  .dstSet{ *descriptor_sets_.sets("geometry").front() },
                  .dstBinding{ descriptor_sets_.binding("geometry", "sampler") },
                  .dstArrayElement{ 0 },
                  .descriptorCount{ 1 },
                  .descriptorType{ vk::DescriptorType::eSampler },
                  .pImageInfo{ &infos.back() }
               });

               writes.push_back({
                  .dstSet{ *descriptor_sets_.sets("hdr").front() },
                  .dstBinding{ descriptor_sets_.binding("hdr", "sampler") },
                  .dstArrayElement{ 0 },
                  .descriptorCount{ 1 },
                  .descriptorType{ vk::DescriptorType::eSampler },
                  .pImageInfo{ &infos.back() }
               });

               auto const create_image_writes{
                  [this, &infos, &writes](std::span<std::pair<Image, ImageView> const> images, std::string_view const set,
                  std::string_view const binding)
                  {
                     for (std::uint32_t index{}; index < images.size(); ++index)
                     {
                        auto const& [image, image_view]{ images[index] };
                        infos.push_back({
                           .imageView{ *image_view.image_view() },
                           .imageLayout{ image.info().initialLayout }
                        });

                        writes.push_back({
                           .dstSet{ *descriptor_sets_.sets(set).front() },
                           .dstBinding{ descriptor_sets_.binding(set, binding) },
                           .dstArrayElement{ index },
                           .descriptorCount{ 1 },
                           .descriptorType{ vk::DescriptorType::eSampledImage },
                           .pImageInfo{ &infos.back() }
                        });
                     }
                  }
               };

               create_image_writes(scene_.base_color_images(), "texturing", "base_color_textures");
               create_image_writes(scene_.normal_images(), "texturing", "normal_textures");
               create_image_writes(scene_.metalness_images(), "texturing", "metalness_textures");

               device_.device().updateDescriptorSets(writes, {});

               return sampler;
            }()
         };

         std::vector<vk::raii::CommandBuffer> command_buffers_{
            device_.device().allocateCommandBuffers({
               .commandPool{ *device_.command_pool(device_.queues().front()) },
               .level{ vk::CommandBufferLevel::ePrimary },
               .commandBufferCount{ FRAMES_IN_FLIGHT }
            })
         };

         std::vector<vk::raii::Semaphore> image_available_semaphores_{
            [this]
            {
               std::vector<vk::raii::Semaphore> semaphores{};
               semaphores.reserve(FRAMES_IN_FLIGHT);
               for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
                  semaphores.emplace_back(device_.device().createSemaphore({}));

               return semaphores;
            }()
         };

         std::vector<vk::raii::Semaphore> render_finished_semaphores_{
            [this]
            {
               std::vector<vk::raii::Semaphore> semaphores{};
               semaphores.reserve(FRAMES_IN_FLIGHT);
               for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
                  semaphores.emplace_back(device_.device().createSemaphore({}));

               return semaphores;
            }()
         };

         std::vector<vk::raii::Fence> command_buffer_executed_fences_{
            [this]
            {
               std::vector<vk::raii::Fence> fences{};
               fences.reserve(FRAMES_IN_FLIGHT);
               for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
                  fences.emplace_back(device_.device().createFence({
                     .flags{ vk::FenceCreateFlagBits::eSignaled }
                  }));

               return fences;
            }()
         };

         std::uint32_t current_frame_{};
   };
}

#endif