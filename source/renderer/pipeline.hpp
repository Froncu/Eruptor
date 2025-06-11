#ifndef PIPELINE_HPP
#define PIPELINE_HPP

namespace eru
{
   class Pipeline
   {
      friend class GraphicsPipelineBuilder;
      friend class ComputePipelineBuilder;

      public:
         Pipeline(Pipeline const&) = delete;
         Pipeline(Pipeline&&) = default;

         ~Pipeline() = default;

         Pipeline& operator=(Pipeline const&) = delete;
         Pipeline& operator=(Pipeline&&) = default;

         [[nodiscard]] vk::raii::Pipeline const& pipeline() const;
         [[nodiscard]] vk::raii::PipelineLayout const& layout() const;

      private:
         Pipeline(vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline);

         vk::raii::PipelineLayout pipeline_layout_;
         vk::raii::Pipeline pipeline_;
   };
}

#endif