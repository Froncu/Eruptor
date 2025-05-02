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

         [[nodiscard]] vk::Pipeline build() const;

      private:
         
   };
}

#endif