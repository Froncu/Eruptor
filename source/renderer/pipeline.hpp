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

      private:
         Pipeline(vk::raii::DescriptorPool descriptor_pool, std::vector<vk::raii::DescriptorSet> descriptor_sets,
            vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline);

         vk::raii::DescriptorPool descriptor_pool_;
         std::vector<vk::raii::DescriptorSet> descriptor_sets_;
         vk::raii::PipelineLayout pipeline_layout_;
         vk::raii::Pipeline pipeline_;
   };
}

#endif