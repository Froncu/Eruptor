#include "pipeline_builder.hpp"

namespace eru
{
   PipelineBuilder& PipelineBuilder::add_shader(vk::PipelineShaderStageCreateInfo const& shader)
   {
      shaders_.emplace_back(shader);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_shaders(std::span<vk::PipelineShaderStageCreateInfo> const shaders)
   {
      for (vk::PipelineShaderStageCreateInfo const shader : shaders)
         add_shader(shader);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_vertex_binding(vk::VertexInputBindingDescription const vertex_binding)
   {
      vertex_bindings_.emplace_back(vertex_binding);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_vertex_bindings(std::span<vk::VertexInputBindingDescription> const vertex_bindings)
   {
      for (vk::VertexInputBindingDescription const vertex_binding : vertex_bindings)
         add_vertex_binding(vertex_binding);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_vertex_attribute(vk::VertexInputAttributeDescription const vertex_attribute)
   {
      vertex_attributes_.emplace_back(vertex_attribute);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_vertex_attributes(std::span<vk::VertexInputAttributeDescription> const vertex_attributes)
   {
      for (vk::VertexInputAttributeDescription const vertex_attribute : vertex_attributes)
         add_vertex_attribute(vertex_attribute);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::change_input_assembly(vk::PipelineInputAssemblyStateCreateInfo const& input_assembly)
   {
      input_assembly_ = input_assembly;
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_dynamic_state(vk::DynamicState const state)
   {
      dynamic_states_.emplace_back(state);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_dynamic_states(std::span<vk::DynamicState> const states)
   {
      for (vk::DynamicState const state : states)
         add_dynamic_state(state);

      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_descriptor_set_layout_binding(vk::DescriptorSetLayoutBinding const binding)
   {
      descriptor_set_layout_bindings_.emplace_back(binding);
      return *this;
   }

   PipelineBuilder& PipelineBuilder::add_descriptor_set_layout_bindings(
      std::span<vk::DescriptorSetLayoutBinding> const bindings)
   {
      for (vk::DescriptorSetLayoutBinding const binding : bindings)
         add_descriptor_set_layout_binding(binding);

      return *this;
   }
}