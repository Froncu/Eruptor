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

      private:
         Pipeline(std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts,
            vk::raii::DescriptorPool descriptor_pool,
            std::unordered_map<std::string, std::vector<vk::raii::DescriptorSet>> descriptor_sets,
            vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline);

         std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts_;
         vk::raii::DescriptorPool descriptor_pool_;
         std::unordered_map<std::string, std::vector<vk::raii::DescriptorSet>> descriptor_sets_;
         vk::raii::PipelineLayout pipeline_layout_;
         vk::raii::Pipeline pipeline_;
   };
}

#endif