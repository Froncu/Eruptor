#include "mesh/vertex.hpp"
#include "pipeline_builder.hpp"
#include "utility/exception.hpp"

namespace eru
{
   PipelineBuilder& PipelineBuilder::change_color_attachment_format(vk::Format const color_attachment_format)
   {
      color_attachment_format_ = color_attachment_format;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_depth_attachment_format(vk::Format const depth_attachment_format)
   {
      depth_attachment_format_ = depth_attachment_format;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_shader_stage(vk::PipelineShaderStageCreateInfo const& shader_stage)
   {
      shader_stages_.emplace_back(shader_stage);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_shader_stages(std::initializer_list<vk::PipelineShaderStageCreateInfo> const shader_stages)
   {
      for (vk::PipelineShaderStageCreateInfo const shader_stage : shader_stages)
         add_shader_stage(shader_stage);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_input_assembly_state(
      vk::PipelineInputAssemblyStateCreateInfo const& input_assembly_state)
   {
      input_assembly_state_ = input_assembly_state;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_rasterization_state(
      vk::PipelineRasterizationStateCreateInfo const& rasterization_state)
   {
      rasterization_state_ = rasterization_state;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_multisample_state(vk::PipelineMultisampleStateCreateInfo const& multisample_state)
   {
      multisample_state_ = multisample_state;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_depth_stencil_state(
      vk::PipelineDepthStencilStateCreateInfo const& depth_stencil_state)
   {
      depth_stencil_state_ = depth_stencil_state;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_color_blend_attachment_state(vk::PipelineColorBlendAttachmentState const& color_blend_attachment_state)
   {
      color_blend_state_ = color_blend_attachment_state;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_dynamic_state(vk::DynamicState const state)
   {
      dynamic_states_.emplace_back(state);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_dynamic_states(std::initializer_list<vk::DynamicState> const states)
   {
      for (vk::DynamicState const state : states)
         add_dynamic_state(state);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_descriptor_binding(DescriptorBinding const descriptor_binding)
   {
      if (not descriptor_binding.count)
         exception("descriptor binding count must be greater than 0!");

      descriptor_bindings_.emplace_back(descriptor_binding);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_descriptor_bindings(std::initializer_list<DescriptorBinding> const descriptor_bindings)
   {
      for (DescriptorBinding const binding : descriptor_bindings)
         add_descriptor_binding(binding);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_descriptor_set_count(std::uint32_t const descriptor_set_count)
   {
      if (not descriptor_set_count)
         exception("descriptor set count must be greater than 0!");

      descriptor_set_count_ = descriptor_set_count;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_push_constant_range(vk::PushConstantRange const push_constant_range)
   {
      push_constant_ranges_.emplace_back(push_constant_range);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_push_constant_ranges(std::initializer_list<vk::PushConstantRange> const push_constant_ranges)
   {
      for (vk::PushConstantRange const push_constant_range : push_constant_ranges)
         add_push_constant_range(push_constant_range);

      return *this;
   }

   Pipeline PipelineBuilder::build(Device const& device) const
   {
      vk::raii::DescriptorSetLayout const descriptor_set_layout{ create_descriptor_set_layout(device) };
      vk::raii::DescriptorPool descriptor_pool{ create_descriptor_pool(device) };
      std::vector descriptor_sets{ allocate_descriptor_sets(device, descriptor_set_layout, descriptor_pool) };
      vk::raii::PipelineLayout pipeline_layout{ create_pipeline_layout(device, descriptor_set_layout) };

      return {
         std::move(descriptor_pool), std::move(descriptor_sets),
         std::move(pipeline_layout), create_pipeline(device, pipeline_layout)
      };
   }

   vk::raii::DescriptorSetLayout PipelineBuilder::create_descriptor_set_layout(Device const& device) const
   {
      if (descriptor_bindings_.empty())
         return nullptr;

      std::vector<vk::DescriptorSetLayoutBinding> descriptor_bindings{};
      descriptor_bindings.reserve(descriptor_bindings_.size());
      for (std::uint32_t index{}; index < descriptor_bindings_.size(); ++index)
      {
         auto const [type, shader_stage_flags, count]{ descriptor_bindings_[index] };
         descriptor_bindings.push_back({
            .binding{ index },
            .descriptorType{ type },
            .descriptorCount{ count },
            .stageFlags{ shader_stage_flags }
         });
      }

      return device.device().createDescriptorSetLayout({
         .bindingCount{ static_cast<std::uint32_t>(descriptor_bindings.size()) },
         .pBindings{ descriptor_bindings.data() }
      });
   }

   vk::raii::DescriptorPool PipelineBuilder::create_descriptor_pool(Device const& device) const
   {
      if (descriptor_bindings_.empty())
         return nullptr;

      std::unordered_map<vk::DescriptorType, std::uint32_t> descriptor_type_sizes{};
      descriptor_type_sizes.reserve(descriptor_bindings_.size());
      for (DescriptorBinding const descriptor_binding : descriptor_bindings_)
         descriptor_type_sizes[descriptor_binding.type] += descriptor_binding.count;

      std::vector<vk::DescriptorPoolSize> descriptor_pool_sizes{};
      descriptor_pool_sizes.reserve(descriptor_type_sizes.size());
      for (auto const [type, count] : descriptor_type_sizes)
         descriptor_pool_sizes.push_back({
            .type{ type },
            .descriptorCount{ descriptor_set_count_ * count },
         });

      return device.device().createDescriptorPool({
         .flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet },
         .maxSets{ descriptor_set_count_ },
         .poolSizeCount{ static_cast<std::uint32_t>(descriptor_pool_sizes.size()) },
         .pPoolSizes{ descriptor_pool_sizes.data() }
      });
   }

   std::vector<vk::raii::DescriptorSet> PipelineBuilder::allocate_descriptor_sets(Device const& device,
      vk::raii::DescriptorSetLayout const& descriptor_set_layout, vk::raii::DescriptorPool const& descriptor_pool) const
   {
      if (not *descriptor_set_layout)
         return {};

      std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{ descriptor_set_count_, descriptor_set_layout };
      return device.device().allocateDescriptorSets({
         .descriptorPool{ *descriptor_pool },
         .descriptorSetCount{ static_cast<std::uint32_t>(descriptor_set_layouts.size()) },
         .pSetLayouts{ descriptor_set_layouts.data() }
      });
   }

   vk::raii::PipelineLayout PipelineBuilder::create_pipeline_layout(Device const& device,
      vk::raii::DescriptorSetLayout const& descriptor_set_layout) const
   {
      return device.device().createPipelineLayout({
         .setLayoutCount{ *descriptor_set_layout ? 1u : 0u },
         .pSetLayouts{ &*descriptor_set_layout },
         .pushConstantRangeCount{ static_cast<std::uint32_t>(push_constant_ranges_.size()) },
         .pPushConstantRanges{ push_constant_ranges_.data() }
      });
   }

   vk::raii::Pipeline PipelineBuilder::create_pipeline(Device const& device,
      vk::raii::PipelineLayout const& pipeline_layout) const
   {
      vk::PipelineRenderingCreateInfo const rendering_create_info{
         .colorAttachmentCount{ 1 },
         .pColorAttachmentFormats{ &color_attachment_format_ },
         .depthAttachmentFormat{ depth_attachment_format_ }
      };

      vk::PipelineVertexInputStateCreateInfo constexpr vertex_input_state_create_info{
         .vertexBindingDescriptionCount{ static_cast<std::uint32_t>(Vertex::BINDING_DESCRIPTIONS.size()) },
         .pVertexBindingDescriptions{ Vertex::BINDING_DESCRIPTIONS.data() },
         .vertexAttributeDescriptionCount{ static_cast<std::uint32_t>(Vertex::ATTRIBUTE_DESCRIPTIONS.size()) },
         .pVertexAttributeDescriptions{ Vertex::ATTRIBUTE_DESCRIPTIONS.data() }
      };

      vk::PipelineViewportStateCreateInfo constexpr viewport_state{
         .viewportCount{ 1 },
         .scissorCount{ 1 }
      };

      vk::PipelineDynamicStateCreateInfo const dynamic_state{
         .dynamicStateCount{ static_cast<std::uint32_t>(dynamic_states_.size()) },
         .pDynamicStates{ dynamic_states_.data() }
      };

      vk::PipelineColorBlendStateCreateInfo const color_blend_state{
         .attachmentCount{ 1 },
         .pAttachments{ &color_blend_state_ }
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
            .pColorBlendState{ &color_blend_state },
            .pDynamicState{ &dynamic_state },
            .layout{ *pipeline_layout }
         }
      );
   }
}