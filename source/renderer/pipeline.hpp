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
         [[nodiscard]] vk::raii::DescriptorSetLayout const& descriptor_set_layout() const;
         [[nodiscard]] std::span<vk::raii::DescriptorSet const> descriptor_sets() const;

      private:
         Pipeline(vk::raii::DescriptorSetLayout descriptor_set_layout, vk::raii::DescriptorPool descriptor_pool,
            std::vector<vk::raii::DescriptorSet> descriptor_sets,
            vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline);

         vk::raii::DescriptorSetLayout descriptor_set_layout_;
         vk::raii::DescriptorPool descriptor_pool_;
         std::vector<vk::raii::DescriptorSet> descriptor_sets_;
         vk::raii::PipelineLayout pipeline_layout_;
         vk::raii::Pipeline pipeline_;
   };
}

#endif