#ifndef PIPELINE_BUILDER_HPP
#define PIPELINE_BUILDER_HPP

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

         PipelineBuilder& add_shader(vk::PipelineShaderStageCreateInfo const& shader);
         PipelineBuilder& add_shaders(std::span<vk::PipelineShaderStageCreateInfo> shaders);
         PipelineBuilder& add_vertex_binding(vk::VertexInputBindingDescription vertex_binding);
         PipelineBuilder& add_vertex_bindings(std::span<vk::VertexInputBindingDescription> vertex_bindings);
         PipelineBuilder& add_vertex_attribute(vk::VertexInputAttributeDescription vertex_attribute);
         PipelineBuilder& add_vertex_attributes(std::span<vk::VertexInputAttributeDescription> vertex_attributes);
         PipelineBuilder& change_input_assembly(vk::PipelineInputAssemblyStateCreateInfo const& input_assembly);
         PipelineBuilder& add_dynamic_state(vk::DynamicState state);
         PipelineBuilder& add_dynamic_states(std::span<vk::DynamicState> states);
         PipelineBuilder& add_descriptor_set_layout_binding(vk::DescriptorSetLayoutBinding binding);
         PipelineBuilder& add_descriptor_set_layout_bindings(std::span<vk::DescriptorSetLayoutBinding> bindings);

         [[nodiscard]] vk::Pipeline build() const;

      private:
         std::vector<vk::PipelineShaderStageCreateInfo> shaders_{};
         std::vector<vk::DynamicState> dynamic_states_{};
         std::vector<vk::VertexInputBindingDescription> vertex_bindings_{};
         std::vector<vk::VertexInputAttributeDescription> vertex_attributes_{};
         vk::PipelineInputAssemblyStateCreateInfo input_assembly_{};
         std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings_{};
   };
}

#endif