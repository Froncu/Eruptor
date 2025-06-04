#include "pipeline.hpp"

#include "utility/exception.hpp"

namespace eru
{
   Pipeline::Pipeline(vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline)
      : pipeline_layout_{ std::move(pipeline_layout) }
      , pipeline_{ std::move(pipeline) }
   {
   }

   vk::raii::Pipeline const& Pipeline::pipeline() const
   {
      return pipeline_;
   }

   vk::raii::PipelineLayout const& Pipeline::layout() const
   {
      return pipeline_layout_;
   }
}