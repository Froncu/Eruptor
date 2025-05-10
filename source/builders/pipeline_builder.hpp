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
            vk::DescriptorType type{};
            vk::ShaderStageFlags shader_stage_flags{};
            std::uint32_t count{ 1 };
         };

         PipelineBuilder() = default;
         PipelineBuilder(PipelineBuilder const&) = delete;
         PipelineBuilder(PipelineBuilder&&) = delete;

         ~PipelineBuilder() = default;

         PipelineBuilder& operator=(PipelineBuilder const&) = delete;
         PipelineBuilder& operator=(PipelineBuilder&&) = delete;

         PipelineBuilder& change_color_attachment_format(vk::Format color_attachment_format);
         PipelineBuilder& change_depth_attachment_format(vk::Format depth_attachment_format);
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
         PipelineBuilder& add_descriptor_binding(DescriptorBinding descriptor_binding);
         PipelineBuilder& add_descriptor_bindings(std::initializer_list<DescriptorBinding> descriptor_bindings);
         PipelineBuilder& change_descriptor_set_count(std::uint32_t descriptor_set_count);
         PipelineBuilder& add_push_constant_range(vk::PushConstantRange push_constant_range);
         PipelineBuilder& add_push_constant_ranges(std::initializer_list<vk::PushConstantRange> push_constant_ranges);

         [[nodiscard]] Pipeline build(Device const& device) const;

      private:
         [[nodiscard]] vk::raii::DescriptorSetLayout create_descriptor_set_layout(Device const& device) const;
         [[nodiscard]] vk::raii::DescriptorPool create_descriptor_pool(Device const& device) const;
         [[nodiscard]] std::vector<vk::raii::DescriptorSet> allocate_descriptor_sets(Device const& device,
            vk::raii::DescriptorSetLayout const& descriptor_set_layout,
            vk::raii::DescriptorPool const& descriptor_pool) const;
         [[nodiscard]] vk::raii::PipelineLayout create_pipeline_layout(Device const& device,
            vk::raii::DescriptorSetLayout const& descriptor_set_layout) const;
         [[nodiscard]] vk::raii::Pipeline create_pipeline(Device const& device,
            vk::raii::PipelineLayout const& pipeline_layout) const;

         vk::Format color_attachment_format_{};
         vk::Format depth_attachment_format_{};
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
         std::vector<DescriptorBinding> descriptor_bindings_{};
         std::uint32_t descriptor_set_count_{ 1 };
         std::vector<vk::PushConstantRange> push_constant_ranges_{};
   };
}

#endif