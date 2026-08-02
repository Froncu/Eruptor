#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "eruptor/api.hpp"
#include "eruptor/layout.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/shader_structure.hpp"
#include "eruptor/vertex.hpp"
#include "eruptor/window.hpp"

struct ktxTexture2;

namespace eru
{
   ERU_SHADER_STRUCTURE(UniformBufferObject)
   {
      ERU_SHADER_FIELD(glm::mat4, model);
      ERU_SHADER_FIELD(glm::mat4, view);
      ERU_SHADER_FIELD(glm::mat4, projection);
   };

   class Renderer final
   {
      public:
         struct FrameData final
         {
            vk::raii::CommandBuffer const& command_buffer;
            std::uint8_t frame_index;
            std::uint8_t frames_in_flight;
         };

         struct Target final
         {
            vk::Image image;
            vk::ImageView image_view;
            vk::Extent2D extent;
            vk::Format format;
         };

         ERU_API explicit Renderer();
         Renderer(Renderer const&) = delete;
         Renderer(Renderer&&) noexcept = delete;

         ERU_API ~Renderer();

         auto operator=(Renderer const&) -> Renderer& = delete;
         auto operator=(Renderer&&) -> Renderer& = delete;

         ERU_API auto record(FrameData frame_data, Target const& target) -> void;

      private:
         [[nodiscard]] auto uniform_buffer_descriptor_set_layout() const -> vk::raii::DescriptorSetLayout;
         [[nodiscard]] auto sampler_descriptor_set_layout() const -> vk::raii::DescriptorSetLayout;
         [[nodiscard]] auto descriptor_pool() const -> vk::raii::DescriptorPool;
         [[nodiscard]] auto uniform_buffer_descriptor_sets() const -> std::vector<vk::raii::DescriptorSet>;
         [[nodiscard]] auto sampler_descriptor_set() const -> vk::raii::DescriptorSet;

         [[nodiscard]] auto pipeline_layout() const -> vk::raii::PipelineLayout;
         [[nodiscard]] auto pipeline() const -> vk::raii::Pipeline;

         [[nodiscard]] auto vertex_buffer() const -> vk::raii::Buffer;
         [[nodiscard]] auto vertex_buffer_memory() const -> vk::raii::DeviceMemory;
         [[nodiscard]] auto index_buffer() const -> vk::raii::Buffer;
         [[nodiscard]] auto index_buffer_memory() const -> vk::raii::DeviceMemory;
         [[nodiscard]] auto uniform_buffers() const -> std::vector<vk::raii::Buffer>;
         [[nodiscard]] auto uniform_buffer_memories() const -> std::vector<vk::raii::DeviceMemory>;
         [[nodiscard]] auto texture(std::string_view path) const -> UniquePointer<ktxTexture2>;
         [[nodiscard]] auto image() const -> vk::raii::Image;
         [[nodiscard]] auto image_view() const -> vk::raii::ImageView;
         [[nodiscard]] auto sampler() const -> vk::raii::Sampler;
         [[nodiscard]] auto image_memory() const -> vk::raii::DeviceMemory;

         [[nodiscard]] auto depth_image() const -> vk::raii::Image;
         [[nodiscard]] auto depth_image_view() const -> vk::raii::ImageView;
         [[nodiscard]] auto depth_image_memory() const -> vk::raii::DeviceMemory;

         std::vector<Vertex> const vertices_{
            { .position = { -0.5f, -0.5f, -0.2f }, .color = { 1.0f, 0.0f, 0.0f }, .texture_coordinate = { 1.0f, 0.0f } },
            { .position = { 0.5f, -0.5f, -0.2 }, .color = { 0.0f, 1.0f, 0.0f }, .texture_coordinate = { 0.0f, 0.0f } },
            { .position = { 0.5f, 0.5f, 0.2f }, .color = { 0.0f, 0.0f, 1.0f }, .texture_coordinate = { 0.0f, 1.0f } },
            { .position = { -0.5f, 0.5f, 0.2f }, .color = { 1.0f, 1.0f, 1.0f }, .texture_coordinate = { 1.0f, 1.0f } },
            { .position = { -0.5f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f }, .texture_coordinate = { 1.0f, 0.0f } },
            { .position = { 0.5f, -0.5f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f }, .texture_coordinate = { 0.0f, 0.0f } },
            { .position = { 0.5f, 0.5f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f }, .texture_coordinate = { 0.0f, 1.0f } },
            { .position = { -0.5f, 0.5f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f }, .texture_coordinate = { 1.0f, 1.0f } }
         };
         std::vector<uint16_t> const indices_{ 0, 1, 3, 2, 4, 5, 7, 6 };

         Context const& context_{ Locator::get<Context>() };

         Layout const global_layout_{
            "global",
            {
               {
                  .capacity{ 3 },
                  .bindings{
                     {
                        .name{ "camera" },
                        .type{ vk::DescriptorType::eUniformBuffer },
                        .capacity{ 1 },
                        .element_type{ ERU_SHADER_TYPE(UniformBufferObject) }
                     }
                  }
               }
            }
         };

         vk::raii::Image depth_image_{ depth_image() };
         vk::raii::DeviceMemory depth_image_memory_{ depth_image_memory() };
         vk::raii::ImageView depth_image_view_{ nullptr };
         vk::raii::DescriptorSetLayout const uniform_buffer_descriptor_set_layout_{ uniform_buffer_descriptor_set_layout() };
         vk::raii::DescriptorSetLayout const sampler_descriptor_set_layout_{ sampler_descriptor_set_layout() };
         vk::raii::PipelineLayout const pipeline_layout_{ pipeline_layout() };
         vk::raii::Pipeline const pipeline_{ pipeline() };
         vk::raii::Buffer const vertex_buffer_{ vertex_buffer() };
         vk::raii::DeviceMemory const vertex_buffer_memory_{ vertex_buffer_memory() };
         vk::raii::Buffer const index_buffer_{ index_buffer() };
         vk::raii::DeviceMemory const index_buffer_memory_{ index_buffer_memory() };
         std::vector<vk::raii::Buffer> uniform_buffers_{ uniform_buffers() };
         std::vector<vk::raii::DeviceMemory> uniform_buffer_memories_{ uniform_buffer_memories() };
         std::vector<UniformBufferObject*> uniform_buffer_mapped_{};
         UniquePointer<ktxTexture2> const texture_{ texture("assets/textures/test.png") };
         vk::raii::Image const image_{ image() };
         vk::raii::ImageView image_view_{ nullptr };
         vk::raii::Sampler const sampler_{ sampler() };
         vk::raii::DeviceMemory const image_memory_{ image_memory() };
         vk::raii::DescriptorPool const descriptor_pool_{ descriptor_pool() };
         std::vector<vk::raii::DescriptorSet> const uniform_buffer_descriptor_sets_{ uniform_buffer_descriptor_sets() };
         vk::raii::DescriptorSet const sampler_descriptor_set_{ sampler_descriptor_set() };
   };
}

#endif