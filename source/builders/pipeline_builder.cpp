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

   PipelineBuilder& PipelineBuilder::add_vertex_binding(vk::VertexInputBindingDescription const& vertex_binding)
   {
      vertex_bindings_.emplace_back(vertex_binding);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_vertex_bindings(
      std::span<vk::VertexInputBindingDescription const> const vertex_bindings)
   {
      for (vk::VertexInputBindingDescription const vertex_binding : vertex_bindings)
         add_vertex_binding(vertex_binding);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_vertex_attribute(vk::VertexInputAttributeDescription const& vertex_attribute)
   {
      vertex_attributes_.emplace_back(vertex_attribute);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_vertex_attributes(
      std::span<vk::VertexInputAttributeDescription const> const vertex_attributes)
   {
      for (vk::VertexInputAttributeDescription const vertex_attribute : vertex_attributes)
         add_vertex_attribute(vertex_attribute);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_shader_stage(vk::PipelineShaderStageCreateInfo const& shader_stage)
   {
      shader_stages_.emplace_back(shader_stage);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_shader_stages(
      std::initializer_list<vk::PipelineShaderStageCreateInfo> const shader_stages)
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

   PipelineBuilder& PipelineBuilder::change_color_blend_attachment_state(
      vk::PipelineColorBlendAttachmentState const& color_blend_attachment_state)
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

   PipelineBuilder& PipelineBuilder::add_descriptor_set(DescriptorSet descriptor_set)
   {
      if (descriptor_set.name.empty())
         exception("the descriptor set name cannot be empty!");

      if (not descriptor_set.bindings.size())
         exception("the descriptor set's \"{}\" bindings cannot be empty!", descriptor_set.name);

      if (std::ranges::any_of(descriptor_set.bindings,
         [](DescriptorBinding const& binding)
         {
            return
               binding.name.empty() or
               binding.count == 0;
         }))
         exception("at least one of the descriptor set's \"{}\" bindings is unnamed or has a count of 0!",
            descriptor_set.name);

      if (std::unordered_set<std::string> names{}; std::ranges::any_of(descriptor_set.bindings,
         [&names](DescriptorBinding const& binding)
         {
            return not names.insert(binding.name).second;
         }))
         exception("all binding names in the descriptor set \"{}\" must be unique!", descriptor_set.name);

      if (not descriptor_set.allocation_count)
         exception("the descriptor set's \"{}\" allocation count cannot be 0!", descriptor_set.name);

      if (auto stored_set{ descriptor_sets_.find(descriptor_set.name) };
         stored_set == descriptor_sets_.end())
      {
         for (stored_set = descriptor_sets_.begin(); stored_set not_eq descriptor_sets_.end(); ++stored_set)
         {
            if (stored_set->bindings == descriptor_set.bindings)
               exception("an already added descriptor set named \"{}\" has identical bindings to \"{}\"!",
                  stored_set->name, descriptor_set.name);
         }

         descriptor_sets_.emplace(std::move(descriptor_set));
      }
      else
         exception("a descriptor set with name \"{}\" already exists!",
            stored_set->name);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_descriptor_sets(std::initializer_list<DescriptorSet> const descriptor_sets)
   {
      for (DescriptorSet const& descriptor_set : descriptor_sets)
         add_descriptor_set(descriptor_set);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::assign_slot_to_descriptor_set(std::string name, std::uint32_t const slot)
   {
      if (name.empty())
         exception("the descriptor set name cannot be empty!");

      if (descriptor_set_slots_.size() <= slot)
         descriptor_set_slots_.resize(slot + 1);

      descriptor_set_slots_[slot] = std::move(name);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::assign_slots_to_descriptor_set(std::string_view const name,
      std::initializer_list<std::uint32_t> const slots)
   {
      for (std::uint32_t const slot : slots)
         assign_slot_to_descriptor_set(name.data(), slot);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_push_constant_range(vk::PushConstantRange const push_constant_range)
   {
      push_constant_ranges_.emplace_back(push_constant_range);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_push_constant_ranges(
      std::initializer_list<vk::PushConstantRange> const push_constant_ranges)
   {
      for (vk::PushConstantRange const push_constant_range : push_constant_ranges)
         add_push_constant_range(push_constant_range);

      return *this;
   }

   Pipeline PipelineBuilder::build(Device const& device)
   {
      std::unordered_map descriptor_set_layouts{ create_descriptor_set_layouts(device) };
      vk::raii::DescriptorPool descriptor_pool{ create_descriptor_pool(device) };
      std::unordered_map descriptor_sets{ allocate_descriptor_sets(device, descriptor_set_layouts, descriptor_pool) };
      vk::raii::PipelineLayout pipeline_layout{ create_pipeline_layout(device, descriptor_set_layouts) };

      std::vector<vk::raii::DescriptorSetLayout> raw_descriptor_set_layouts{};
      raw_descriptor_set_layouts.reserve(descriptor_set_layouts.size());
      for (auto& [layout, binding_map] : std::views::values(descriptor_set_layouts))
         raw_descriptor_set_layouts.emplace_back(std::move(layout));

      return {
         std::move(raw_descriptor_set_layouts), std::move(descriptor_pool), std::move(descriptor_sets),
         std::move(pipeline_layout), create_pipeline(device, pipeline_layout)
      };
   }

   PipelineBuilder::DescriptorSetLayouts PipelineBuilder::create_descriptor_set_layouts(Device const& device) const
   {
      DescriptorSetLayouts layouts{};
      layouts.reserve(descriptor_sets_.size());
      for (auto const& [set_name, bindings, allocation_count] : descriptor_sets_)
      {
         if (std::ranges::find(descriptor_set_slots_, set_name) == descriptor_set_slots_.end())
            exception("descriptor set with name \"{}\" was never assigned a slot!", set_name);

         std::vector<vk::DescriptorSetLayoutBinding> native_bindings{};
         native_bindings.reserve(bindings.size());
         Pipeline::DescriptorSetBindingMap binding_map{};
         binding_map.reserve(bindings.size());
         std::vector<vk::DescriptorBindingFlags> flags_collection{};
         flags_collection.reserve(bindings.size());
         for (std::uint32_t index{}; index < bindings.size(); ++index)
         {
            auto const& [binding_name, flags, type, shader_stage_flags, count]{ bindings[index] };
            native_bindings.push_back({
               .binding{ index },
               .descriptorType{ type },
               .descriptorCount{ count },
               .stageFlags{ shader_stage_flags }
            });

            binding_map.emplace(binding_name, index);

            flags_collection.emplace_back(flags);
         }

         std::uint32_t const binding_count{ static_cast<std::uint32_t>(bindings.size()) };
         vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_create_info{
            .bindingCount{ binding_count },
            .pBindingFlags{ flags_collection.data() }
         };

         layouts.emplace(set_name,
            std::pair{
               device.device().createDescriptorSetLayout({
                  .pNext{ &binding_flags_create_info },
                  .bindingCount{ binding_count },
                  .pBindings{ native_bindings.data() }
               }),
               std::move(binding_map)
            }
         );
      }

      return layouts;
   }

   vk::raii::DescriptorPool PipelineBuilder::create_descriptor_pool(Device const& device) const
   {
      std::unordered_map<vk::DescriptorType, std::uint32_t> type_sizes{};
      std::uint32_t total_set_count{};
      for (auto const& [name, bindings, allocation_count] : descriptor_sets_)
      {
         for (DescriptorBinding const binding : bindings)
            type_sizes[binding.type] += binding.count * allocation_count;

         total_set_count += allocation_count;
      }

      std::vector<vk::DescriptorPoolSize> pool_size{};
      for (auto const& [type, size] : type_sizes)
         pool_size.emplace_back(type, size);

      return device.device().createDescriptorPool({
         .flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet },
         .maxSets{ total_set_count },
         .poolSizeCount{ static_cast<std::uint32_t>(pool_size.size()) },
         .pPoolSizes{ pool_size.data() }
      });
   }

   Pipeline::DescriptorSets PipelineBuilder::allocate_descriptor_sets(Device const& device, DescriptorSetLayouts& layouts,
      vk::raii::DescriptorPool const& pool) const
   {
      Pipeline::DescriptorSets descriptor_sets{};
      for (auto& [name, layout] : layouts)
      {
         std::vector<vk::DescriptorSetLayout> raw_layouts{ descriptor_sets_.find(name)->allocation_count, layout.first };
         descriptor_sets[name] = {
            device.device().allocateDescriptorSets({
               .descriptorPool{ *pool },
               .descriptorSetCount{ static_cast<std::uint32_t>(raw_layouts.size()) },
               .pSetLayouts{ raw_layouts.data() }
            }),
            std::move(layout.second)
         };
      }

      return descriptor_sets;
   }

   vk::raii::PipelineLayout PipelineBuilder::create_pipeline_layout(Device const& device,
      DescriptorSetLayouts const& descriptor_set_layouts)
   {
      if (std::ranges::any_of(descriptor_set_slots_,
         [](std::string_view const name)
         {
            return name.empty();
         }))
         exception("there cannot be a gap between descriptor set slots!");

      std::vector<vk::DescriptorSetLayout> raw_descriptor_set_layouts{};
      raw_descriptor_set_layouts.reserve(descriptor_set_slots_.size());
      for (std::string const& name : descriptor_set_slots_)
      {
         auto const layout{ descriptor_set_layouts.find(name) };
         if (layout == descriptor_set_layouts.end())
            exception("descriptor set with name \"{}\" was assigned a slot but was never added!", name);

         raw_descriptor_set_layouts.push_back(layout->second.first);
      }

      return device.device().createPipelineLayout({
         .setLayoutCount{ static_cast<std::uint32_t>(raw_descriptor_set_layouts.size()) },
         .pSetLayouts{ raw_descriptor_set_layouts.data() },
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