#ifndef PIPELINE_BUILDER_HPP
#define PIPELINE_BUILDER_HPP

#include "renderer/descriptor_sets.hpp"
#include "renderer/device.hpp"
#include "renderer/pipeline.hpp"

namespace eru
{
   class PipelineBuilder final
   {
      public:
         PipelineBuilder() = default;
         PipelineBuilder(PipelineBuilder const&) = delete;
         PipelineBuilder(PipelineBuilder&&) = delete;

         ~PipelineBuilder() = default;

         PipelineBuilder& operator=(PipelineBuilder const&) = delete;
         PipelineBuilder& operator=(PipelineBuilder&&) = delete;

         PipelineBuilder& add_color_attachment_format(vk::Format color_attachment_format);
         PipelineBuilder& add_color_attachment_formats(std::initializer_list<vk::Format> color_attachment_formats);
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
         PipelineBuilder& assign_descriptor_set_layout(std::string name, std::uint32_t slot);
         PipelineBuilder& assign_descriptor_set_layout(std::string_view name, std::initializer_list<std::uint32_t> slots);
         PipelineBuilder& add_push_constant_range(vk::PushConstantRange push_constant_range);
         PipelineBuilder& add_push_constant_ranges(std::initializer_list<vk::PushConstantRange> push_constant_ranges);

         [[nodiscard]] Pipeline build(Device const& device, DescriptorSets const& desriptor_sets);

      private:
         [[nodiscard]] vk::raii::PipelineLayout create_pipeline_layout(Device const& device,
            DescriptorSets const& desriptor_sets);
         [[nodiscard]] vk::raii::Pipeline create_pipeline(Device const& device,
            vk::raii::PipelineLayout const& pipeline_layout) const;

         std::vector<vk::Format> color_attachment_formats_{};
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
         std::vector<std::string> descriptor_set_slots_{};
         std::vector<vk::PushConstantRange> push_constant_ranges_{};
   };
}

#endif