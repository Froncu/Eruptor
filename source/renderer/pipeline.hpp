#ifndef PIPELINE_HPP
#define PIPELINE_HPP

namespace eru
{
   class Pipeline
   {
      friend class PipelineBuilder;

      public:
         Pipeline(Pipeline const&) = delete;
         Pipeline(Pipeline&&) = default;

         ~Pipeline() = default;

         Pipeline& operator=(Pipeline const&) = delete;
         Pipeline& operator=(Pipeline&&) = default;

         [[nodiscard]] vk::raii::Pipeline const& pipeline() const;
         [[nodiscard]] vk::raii::PipelineLayout const& layout() const;
         [[nodiscard]] std::span<vk::raii::DescriptorSet const> descriptor_sets(std::string_view name) const;
         [[nodiscard]] std::uint32_t descriptor_binding(std::string_view set_name, std::string_view binding_name) const;

      private:
         using RawDescriptorSets = std::vector<vk::raii::DescriptorSet>;
         using DescriptorSetBindingMap = std::unordered_map<std::string, std::uint32_t>;
         using DescriptorSets = std::unordered_map<std::string, std::pair<RawDescriptorSets, DescriptorSetBindingMap>>;

         Pipeline(std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts, vk::raii::DescriptorPool descriptor_pool,
            DescriptorSets descriptor_sets, vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline);

         std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts_;
         vk::raii::DescriptorPool descriptor_pool_;
         DescriptorSets descriptor_sets_;
         vk::raii::PipelineLayout pipeline_layout_;
         vk::raii::Pipeline pipeline_;
   };
}

#endif