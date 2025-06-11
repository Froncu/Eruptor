#include "compute_pipeline_builder.hpp"
#include "utility/exception.hpp"

namespace eru
{
   ComputePipelineBuilder& ComputePipelineBuilder::change_shader_stage(vk::PipelineShaderStageCreateInfo const& shader_stage)
   {
      shader_stage_ = shader_stage;
      return *this;
   }

   ComputePipelineBuilder& ComputePipelineBuilder::assign_descriptor_set_layout(std::string name, std::uint32_t slot)
   {
      if (name.empty())
         exception("the descriptor set name cannot be empty!");

      if (descriptor_set_slots_.size() <= slot)
         descriptor_set_slots_.resize(slot + 1);

      descriptor_set_slots_[slot] = std::move(name);
      return *this;
   }

   ComputePipelineBuilder& ComputePipelineBuilder::assign_descriptor_set_layout(std::string_view const name,
      std::initializer_list<std::uint32_t> const slots)
   {
      for (std::uint32_t const slot : slots)
         assign_descriptor_set_layout(name.data(), slot);

      return *this;
   }

   ComputePipelineBuilder& ComputePipelineBuilder::add_push_constant_range(vk::PushConstantRange const push_constant_range)
   {
      push_constant_ranges_.emplace_back(push_constant_range);
      return *this;
   }

   ComputePipelineBuilder& ComputePipelineBuilder::add_push_constant_ranges(
      std::initializer_list<vk::PushConstantRange> const push_constant_ranges)
   {
      for (vk::PushConstantRange const push_constant_range : push_constant_ranges)
         add_push_constant_range(push_constant_range);

      return *this;
   }

   Pipeline ComputePipelineBuilder::build(Device const& device, DescriptorSets const& desriptor_sets)
   {
      vk::raii::PipelineLayout pipeline_layout{ create_pipeline_layout(device, desriptor_sets) };
      return { std::move(pipeline_layout), create_pipeline(device, pipeline_layout) };
   }

   vk::raii::PipelineLayout ComputePipelineBuilder::create_pipeline_layout(Device const& device,
      DescriptorSets const& desriptor_sets)
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

   vk::raii::Pipeline ComputePipelineBuilder::create_pipeline(Device const& device,
      vk::raii::PipelineLayout const& pipeline_layout) const
   {
      return device.device().createComputePipeline(nullptr,
         {
            .stage{ shader_stage_ },
            .layout{ *pipeline_layout }
         });
   }
}