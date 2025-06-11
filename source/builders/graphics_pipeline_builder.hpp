#ifndef GRAPHICS_PIPELINE_BUILDER_HPP
#define GRAPHICS_PIPELINE_BUILDER_HPP

#include "renderer/descriptor_sets.hpp"
#include "renderer/device.hpp"
#include "renderer/pipeline.hpp"

namespace eru
{
   class GraphicsPipelineBuilder final
   {
      public:
         GraphicsPipelineBuilder() = default;
         GraphicsPipelineBuilder(GraphicsPipelineBuilder const&) = delete;
         GraphicsPipelineBuilder(GraphicsPipelineBuilder&&) = delete;

         ~GraphicsPipelineBuilder() = default;

         GraphicsPipelineBuilder& operator=(GraphicsPipelineBuilder const&) = delete;
         GraphicsPipelineBuilder& operator=(GraphicsPipelineBuilder&&) = delete;

         GraphicsPipelineBuilder& add_color_attachment_format(vk::Format color_attachment_format);
         GraphicsPipelineBuilder& add_color_attachment_formats(std::span<vk::Format const> color_attachment_formats);
         GraphicsPipelineBuilder& change_depth_attachment_format(vk::Format depth_attachment_format);
         GraphicsPipelineBuilder& add_vertex_binding(vk::VertexInputBindingDescription const& vertex_binding);
         GraphicsPipelineBuilder& add_vertex_bindings(std::span<vk::VertexInputBindingDescription const> vertex_bindings);
         GraphicsPipelineBuilder& add_vertex_attribute(vk::VertexInputAttributeDescription const& vertex_attribute);
         GraphicsPipelineBuilder& add_vertex_attributes(std::span<vk::VertexInputAttributeDescription const> vertex_attributes);
         GraphicsPipelineBuilder& add_shader_stage(vk::PipelineShaderStageCreateInfo const& shader_stage);
         GraphicsPipelineBuilder& add_shader_stages(std::initializer_list<vk::PipelineShaderStageCreateInfo> shader_stages);
         GraphicsPipelineBuilder& change_input_assembly_state(vk::PipelineInputAssemblyStateCreateInfo const& input_assembly_state);
         GraphicsPipelineBuilder& change_rasterization_state(vk::PipelineRasterizationStateCreateInfo const& rasterization_state);
         GraphicsPipelineBuilder& change_multisample_state(vk::PipelineMultisampleStateCreateInfo const& multisample_state);
         GraphicsPipelineBuilder& change_depth_stencil_state(vk::PipelineDepthStencilStateCreateInfo const& depth_stencil_state);
         GraphicsPipelineBuilder& add_color_blend_attachment_state(
            vk::PipelineColorBlendAttachmentState const& color_blend_attachment_state, std::uint32_t count = 1);
         GraphicsPipelineBuilder& add_color_blend_attachment_states(
            std::initializer_list<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states);
         GraphicsPipelineBuilder& add_dynamic_state(vk::DynamicState state);
         GraphicsPipelineBuilder& add_dynamic_states(std::initializer_list<vk::DynamicState> states);
         GraphicsPipelineBuilder& assign_descriptor_set_layout(std::string name, std::uint32_t slot);
         GraphicsPipelineBuilder& assign_descriptor_set_layout(std::string_view name, std::initializer_list<std::uint32_t> slots);
         GraphicsPipelineBuilder& add_push_constant_range(vk::PushConstantRange push_constant_range);
         GraphicsPipelineBuilder& add_push_constant_ranges(std::initializer_list<vk::PushConstantRange> push_constant_ranges);

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
         std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states_{
            {
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
            }
         };
         std::vector<vk::DynamicState> dynamic_states_{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
         std::vector<std::string> descriptor_set_slots_{};
         std::vector<vk::PushConstantRange> push_constant_ranges_{};
   };
}

#endif