#include "pipeline.hpp"

#include "utility/exception.hpp"

namespace eru
{
   Pipeline::Pipeline(std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts,
      vk::raii::DescriptorPool descriptor_pool,
      std::unordered_map<std::string, std::vector<vk::raii::DescriptorSet>> descriptor_sets,
      vk::raii::PipelineLayout pipeline_layout, vk::raii::Pipeline pipeline)
      : descriptor_set_layouts_{ std::move(descriptor_set_layouts) }
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

   std::span<vk::raii::DescriptorSet const> Pipeline::descriptor_sets(std::string_view name) const
   {
      auto const descriptor_set{ descriptor_sets_.find(name.data()) };
      if (descriptor_set == descriptor_sets_.end())
         exception("descriptor set with name \"{}\" does not exist!", name);

      return descriptor_set->second;
   }
}