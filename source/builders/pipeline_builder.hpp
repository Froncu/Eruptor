#ifndef PIPELINE_BUILDER_HPP
#define PIPELINE_BUILDER_HPP

#include "renderer/device.hpp"
#include "renderer/pipeline.hpp"

namespace eru
{
   class PipelineBuilder final
   {
      public:
         struct DescriptorBinding final
         {
            std::string name{};
            vk::DescriptorBindingFlags flags{};
            vk::DescriptorType type{ vk::DescriptorType::eUniformBuffer };
            vk::ShaderStageFlags shader_stage_flags{ vk::ShaderStageFlagBits::eFragment };
            std::uint32_t count{ 1 };

            [[nodiscard]] bool operator==(DescriptorBinding const& other) const
            {
               return flags == other.flags and
                  type == other.type and
                  shader_stage_flags == other.shader_stage_flags and
                  count == other.count;
            };
         };

         struct DescriptorSet final
         {
            std::string name{};
            std::vector<DescriptorBinding> bindings{};
            std::uint32_t allocation_count{ 1 };

            struct Equal final
            {
               using is_transparent = void;

               [[nodiscard]] bool operator()(DescriptorSet const& a, DescriptorSet const& b) const
               {
                  return a.name == b.name;
               }

               [[nodiscard]] bool operator()(std::string_view const name, DescriptorSet const& set) const
               {
                  return name == set.name;
               }

               [[nodiscard]] bool operator()(DescriptorSet const& set, std::string_view const name) const
               {
                  return set.name == name;
               }
            };

            struct Hasher final
            {
               using is_transparent = void;

               [[nodiscard]] std::size_t operator()(DescriptorSet const& set) const
               {
                  return std::hash<std::string>{}(set.name);
               }

               [[nodiscard]] std::size_t operator()(std::string_view const name) const
               {
                  return std::hash<std::string_view>{}(name);
               }

               [[nodiscard]] std::size_t operator()(std::string const& name) const
               {
                  return std::hash<std::string>{}(name);
               }
            };
         };

         PipelineBuilder() = default;
         PipelineBuilder(PipelineBuilder const&) = delete;
         PipelineBuilder(PipelineBuilder&&) = delete;

         ~PipelineBuilder() = default;

         PipelineBuilder& operator=(PipelineBuilder const&) = delete;
         PipelineBuilder& operator=(PipelineBuilder&&) = delete;

         PipelineBuilder& change_color_attachment_format(vk::Format color_attachment_format);
         PipelineBuilder& change_depth_attachment_format(vk::Format depth_attachment_format);
         PipelineBuilder& add_vertex_binding(vk::VertexInputBindingDescription const& vertex_binding);
         PipelineBuilder& add_vertex_bindings(std::span<vk::VertexInputBindingDescription const> vertex_bindings);
         PipelineBuilder& add_vertex_attribute(vk::VertexInputAttributeDescription const& vertex_attribute);
         PipelineBuilder& add_vertex_attributes(std::span<vk::VertexInputAttributeDescription const> vertex_attributes);
         PipelineBuilder& add_shader_stage(vk::PipelineShaderStageCreateInfo const& shader_stage);
         PipelineBuilder& add_shader_stages(std::initializer_list<vk::PipelineShaderStageCreateInfo> shader_stages);
         PipelineBuilder& change_input_assembly_state(vk::PipelineInputAssemblyStateCreateInfo const& input_assembly_state);
         PipelineBuilder& change_rasterization_state(vk::PipelineRasterizationStateCreateInfo const& rasterization_state);
         PipelineBuilder& change_multisample_state(vk::PipelineMultisampleStateCreateInfo const& multisample_state);
         PipelineBuilder& change_depth_stencil_state(vk::PipelineDepthStencilStateCreateInfo const& depth_stencil_state);
         PipelineBuilder& change_color_blend_attachment_state(
            vk::PipelineColorBlendAttachmentState const& color_blend_attachment_state);
         PipelineBuilder& add_dynamic_state(vk::DynamicState state);
         PipelineBuilder& add_dynamic_states(std::initializer_list<vk::DynamicState> states);
         PipelineBuilder& add_descriptor_set(DescriptorSet descriptor_set);
         PipelineBuilder& add_descriptor_sets(std::initializer_list<DescriptorSet> descriptor_sets);
         PipelineBuilder& assign_slot_to_descriptor_set(std::string name, std::uint32_t slot);
         PipelineBuilder& assign_slots_to_descriptor_set(std::string_view name, std::initializer_list<std::uint32_t> slots);
         PipelineBuilder& add_push_constant_range(vk::PushConstantRange push_constant_range);
         PipelineBuilder& add_push_constant_ranges(std::initializer_list<vk::PushConstantRange> push_constant_ranges);

         [[nodiscard]] Pipeline build(Device const& device);

      private:
         using DescriptorSetLayouts =
         std::unordered_map<std::string, std::pair<vk::raii::DescriptorSetLayout, Pipeline::DescriptorSetBindingMap>>;

         [[nodiscard]] DescriptorSetLayouts create_descriptor_set_layouts(Device const& device) const;
         [[nodiscard]] vk::raii::DescriptorPool create_descriptor_pool(Device const& device) const;
         [[nodiscard]] Pipeline::DescriptorSets allocate_descriptor_sets(Device const& device, DescriptorSetLayouts& layouts,
            vk::raii::DescriptorPool const& pool) const;
         [[nodiscard]] vk::raii::PipelineLayout create_pipeline_layout(Device const& device,
            DescriptorSetLayouts const& descriptor_set_layouts);
         [[nodiscard]] vk::raii::Pipeline create_pipeline(Device const& device,
            vk::raii::PipelineLayout const& pipeline_layout) const;

         vk::Format color_attachment_format_{};
         vk::Format depth_attachment_format_{};
         std::vector<vk::VertexInputBindingDescription> vertex_bindings_{};
         std::vector<vk::VertexInputAttributeDescription> vertex_attributes_{};
         std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_{};
         vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_{
            .topology{ vk::PrimitiveTopology::eTriangleList }
         };
         vk::PipelineRasterizationStateCreateInfo rasterization_state_{
            .polygonMode{ vk::PolygonMode::eFill },
            .cullMode{ vk::CullModeFlagBits::eBack },
            .frontFace{ vk::FrontFace::eClockwise },
            .lineWidth{ 1.0f }
         };
         vk::PipelineMultisampleStateCreateInfo multisample_state_{
            .rasterizationSamples{ vk::SampleCountFlagBits::e1 }
         };
         vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_{
            .depthTestEnable{ true },
            .depthWriteEnable{ true },
            .depthCompareOp{ vk::CompareOp::eLess },
         };
         vk::PipelineColorBlendAttachmentState color_blend_state_{
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
         };
         std::vector<vk::DynamicState> dynamic_states_{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
         std::unordered_set<DescriptorSet, DescriptorSet::Hasher, DescriptorSet::Equal> descriptor_sets_{};
         std::vector<std::string> descriptor_set_slots_{};
         std::vector<vk::PushConstantRange> push_constant_ranges_{};
   };
}

#endif