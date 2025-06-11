#ifndef COMPUTE_PIPELINE_BUILDER_HPP
#define COMPUTE_PIPELINE_BUILDER_HPP

#include "erupch/erupch.hpp"
#include "renderer/descriptor_sets.hpp"
#include "renderer/device.hpp"
#include "renderer/pipeline.hpp"

namespace eru
{
   class ComputePipelineBuilder final
   {
      public:
         ComputePipelineBuilder& change_shader_stage(vk::PipelineShaderStageCreateInfo const& shader_stage);
         ComputePipelineBuilder& assign_descriptor_set_layout(std::string name, std::uint32_t slot);
         ComputePipelineBuilder& assign_descriptor_set_layout(std::string_view name, std::initializer_list<std::uint32_t> slots);
         ComputePipelineBuilder& add_push_constant_range(vk::PushConstantRange push_constant_range);
         ComputePipelineBuilder& add_push_constant_ranges(std::initializer_list<vk::PushConstantRange> push_constant_ranges);

         Pipeline build(Device const& device, DescriptorSets const& desriptor_sets);

      private:
         [[nodiscard]] vk::raii::PipelineLayout create_pipeline_layout(Device const& device,
            DescriptorSets const& desriptor_sets);
         [[nodiscard]] vk::raii::Pipeline create_pipeline(Device const& device,
            vk::raii::PipelineLayout const& pipeline_layout) const;

         vk::PipelineShaderStageCreateInfo shader_stage_{};
         std::vector<std::string> descriptor_set_slots_{};
         std::vector<vk::PushConstantRange> push_constant_ranges_{};
   };
}

#endif