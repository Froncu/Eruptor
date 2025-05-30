#include "pipeline.hpp"

namespace eru
{
   Pipeline::Pipeline(vk::raii::DescriptorSetLayout descriptor_set_layout, vk::raii::DescriptorPool descriptor_pool,
      std::vector<vk::raii::DescriptorSet> descriptor_sets,
      vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline)
      : descriptor_set_layout_{ std::move(descriptor_set_layout) }
      , descriptor_pool_{ std::move(descriptor_pool) }
      , descriptor_sets_{ std::move(descriptor_sets) }
      , pipeline_layout_{ std::move(pipeline_layout) }
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

   vk::raii::DescriptorSetLayout const& Pipeline::descriptor_set_layout() const
   {
      return descriptor_set_layout_;
   }

   std::span<vk::raii::DescriptorSet const> Pipeline::descriptor_sets() const
   {
      return descriptor_sets_;
   }
}