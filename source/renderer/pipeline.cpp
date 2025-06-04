#include "pipeline.hpp"

#include "utility/exception.hpp"

namespace eru
{
   Pipeline::Pipeline(std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts,
      vk::raii::DescriptorPool descriptor_pool, DescriptorSets descriptor_sets, vk::raii::PipelineLayout pipeline_layout,
      vk::raii::Pipeline pipeline)
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
      auto const set{ descriptor_sets_.find(name.data()) };
      if (set == descriptor_sets_.end())
         exception("descriptor set with name \"{}\" does not exist!", name);

      return set->second.first;
   }

   std::uint32_t Pipeline::descriptor_binding(std::string_view set_name, std::string_view binding_name) const
   {
      auto const set{ descriptor_sets_.find(set_name.data()) };
      if (set == descriptor_sets_.end())
         exception("descriptor set with name \"{}\" does not exist!", set_name);

      auto const binding{ set->second.second.find(binding_name.data()) };
      if (binding == set->second.second.end())
         exception("descriptor binding with name \"{}\" does not exist in descriptor set \"{}\"!",
            binding_name, set_name);

      return binding->second;
   }
}