#include "graphics_pipeline_builder.hpp"
#include "utility/exception.hpp"

namespace eru
{
   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_color_attachment_format(vk::Format const color_attachment_format)
   {
      color_attachment_formats_.emplace_back(color_attachment_format);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_color_attachment_formats(std::span<vk::Format const> const color_attachment_formats)
   {
      for (vk::Format const color_attachment_format : color_attachment_formats)
         add_color_attachment_format(color_attachment_format);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::change_depth_attachment_format(vk::Format const depth_attachment_format)
   {
      depth_attachment_format_ = depth_attachment_format;
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_vertex_binding(vk::VertexInputBindingDescription const& vertex_binding)
   {
      vertex_bindings_.emplace_back(vertex_binding);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_vertex_bindings(
      std::span<vk::VertexInputBindingDescription const> const vertex_bindings)
   {
      for (vk::VertexInputBindingDescription const vertex_binding : vertex_bindings)
         add_vertex_binding(vertex_binding);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_vertex_attribute(vk::VertexInputAttributeDescription const& vertex_attribute)
   {
      vertex_attributes_.emplace_back(vertex_attribute);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_vertex_attributes(
      std::span<vk::VertexInputAttributeDescription const> const vertex_attributes)
   {
      for (vk::VertexInputAttributeDescription const vertex_attribute : vertex_attributes)
         add_vertex_attribute(vertex_attribute);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_shader_stage(vk::PipelineShaderStageCreateInfo const& shader_stage)
   {
      shader_stages_.emplace_back(shader_stage);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_shader_stages(
      std::initializer_list<vk::PipelineShaderStageCreateInfo> const shader_stages)
   {
      for (vk::PipelineShaderStageCreateInfo const shader_stage : shader_stages)
         add_shader_stage(shader_stage);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::change_input_assembly_state(
      vk::PipelineInputAssemblyStateCreateInfo const& input_assembly_state)
   {
      input_assembly_state_ = input_assembly_state;
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::change_rasterization_state(
      vk::PipelineRasterizationStateCreateInfo const& rasterization_state)
   {
      rasterization_state_ = rasterization_state;
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::change_multisample_state(vk::PipelineMultisampleStateCreateInfo const& multisample_state)
   {
      multisample_state_ = multisample_state;
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::change_depth_stencil_state(
      vk::PipelineDepthStencilStateCreateInfo const& depth_stencil_state)
   {
      depth_stencil_state_ = depth_stencil_state;
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_color_blend_attachment_state(
      vk::PipelineColorBlendAttachmentState const& color_blend_attachment_state, std::uint32_t count)
   {
      while (count--)
         color_blend_attachment_states_.emplace_back(color_blend_attachment_state);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_color_blend_attachment_states(
      std::initializer_list<vk::PipelineColorBlendAttachmentState> const color_blend_attachment_states)
   {
      for (vk::PipelineColorBlendAttachmentState const& color_blend_attachment_state : color_blend_attachment_states)
         add_color_blend_attachment_state(color_blend_attachment_state);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::change_color_blend_attachment_state(
      vk::PipelineColorBlendAttachmentState const& color_blend_attachment_state, std::uint32_t const count)
   {
      color_blend_attachment_states_.assign(count, color_blend_attachment_state);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::change_color_blend_attachment_states(
      std::initializer_list<vk::PipelineColorBlendAttachmentState> const color_blend_attachment_states)
   {
      color_blend_attachment_states_ = color_blend_attachment_states;
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_dynamic_state(vk::DynamicState const state)
   {
      dynamic_states_.emplace_back(state);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_dynamic_states(std::initializer_list<vk::DynamicState> const states)
   {
      for (vk::DynamicState const state : states)
         add_dynamic_state(state);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::assign_descriptor_set_layout(std::string name, std::uint32_t slot)
   {
      if (name.empty())
         exception("the descriptor set name cannot be empty!");

      if (descriptor_set_slots_.size() <= slot)
         descriptor_set_slots_.resize(slot + 1);

      descriptor_set_slots_[slot] = std::move(name);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::assign_descriptor_set_layout(std::string_view const name,
      std::initializer_list<std::uint32_t> const slots)
   {
      for (std::uint32_t const slot : slots)
         assign_descriptor_set_layout(name.data(), slot);

      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_push_constant_range(vk::PushConstantRange const push_constant_range)
   {
      push_constant_ranges_.emplace_back(push_constant_range);
      return *this;
   }

   GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_push_constant_ranges(
      std::initializer_list<vk::PushConstantRange> const push_constant_ranges)
   {
      for (vk::PushConstantRange const push_constant_range : push_constant_ranges)
         add_push_constant_range(push_constant_range);

      return *this;
   }

   Pipeline GraphicsPipelineBuilder::build(Device const& device, DescriptorSets const& desriptor_sets)
   {
      vk::raii::PipelineLayout pipeline_layout{ create_pipeline_layout(device, desriptor_sets) };
      return { std::move(pipeline_layout), create_pipeline(device, pipeline_layout) };
   }

   vk::raii::PipelineLayout GraphicsPipelineBuilder::create_pipeline_layout(Device const& device, DescriptorSets const& desriptor_sets)
   {
      if (std::ranges::any_of(descriptor_set_slots_,
         [](std::string_view const name)
         {
            return name.empty();
         }))
         exception("there cannot be a gap between descriptor set slots!");

      std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{};
      descriptor_set_layouts.reserve(descriptor_set_slots_.size());
      for (std::string const& name : descriptor_set_slots_)
         descriptor_set_layouts.push_back(desriptor_sets.layout(name));

      return device.device().createPipelineLayout({
         .setLayoutCount{ static_cast<std::uint32_t>(descriptor_set_layouts.size()) },
         .pSetLayouts{ descriptor_set_layouts.data() },
         .pushConstantRangeCount{ static_cast<std::uint32_t>(push_constant_ranges_.size()) },
         .pPushConstantRanges{ push_constant_ranges_.data() }
      });
   }

   vk::raii::Pipeline GraphicsPipelineBuilder::create_pipeline(Device const& device,
      vk::raii::PipelineLayout const& pipeline_layout) const
   {
      vk::PipelineRenderingCreateInfo const rendering_create_info{
         .colorAttachmentCount{ static_cast<std::uint32_t>(color_attachment_formats_.size()) },
         .pColorAttachmentFormats{ color_attachment_formats_.data() },
         .depthAttachmentFormat{ depth_attachment_format_ },
      };

      vk::PipelineVertexInputStateCreateInfo const vertex_input_state_create_info{
         .vertexBindingDescriptionCount{ static_cast<std::uint32_t>(vertex_bindings_.size()) },
         .pVertexBindingDescriptions{ vertex_bindings_.data() },
         .vertexAttributeDescriptionCount{ static_cast<std::uint32_t>(vertex_attributes_.size()) },
         .pVertexAttributeDescriptions{ vertex_attributes_.data() }
      };

      vk::PipelineViewportStateCreateInfo constexpr viewport_state{
         .viewportCount{ 1 },
         .scissorCount{ 1 }
      };

      vk::PipelineDynamicStateCreateInfo const dynamic_state{
         .dynamicStateCount{ static_cast<std::uint32_t>(dynamic_states_.size()) },
         .pDynamicStates{ dynamic_states_.data() }
      };

      vk::PipelineColorBlendStateCreateInfo const color_blend_attachment_state{
         .attachmentCount{ static_cast<std::uint32_t>(color_blend_attachment_states_.size()) },
         .pAttachments{ color_blend_attachment_states_.data() }
      };

      return device.device().createGraphicsPipeline(nullptr, {
            .pNext{ &rendering_create_info },
            .stageCount{ static_cast<std::uint32_t>(shader_stages_.size()) },
            .pStages{ shader_stages_.data() },
            .pVertexInputState{ &vertex_input_state_create_info },
            .pInputAssemblyState{ &input_assembly_state_ },
            .pViewportState{ &viewport_state },
            .pRasterizationState{ &rasterization_state_ },
            .pMultisampleState{ &multisample_state_ },
            .pDepthStencilState{ &depth_stencil_state_ },
            .pColorBlendState{ &color_blend_attachment_state },
            .pDynamicState{ &dynamic_state },
            .layout{ *pipeline_layout }
         }
      );
   }
}