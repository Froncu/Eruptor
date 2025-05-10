#include "pipeline.hpp"

namespace eru
{
   Pipeline::Pipeline(vk::raii::DescriptorPool descriptor_pool, std::vector<vk::raii::DescriptorSet> descriptor_sets,
      vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline)
      : descriptor_pool_{ std::move(descriptor_pool) }
      , descriptor_sets_{ std::move(descriptor_sets) }
      , pipeline_layout_{ std::move(pipeline_layout) }
      , pipeline_{ std::move(pipeline) }
   {
   }
}