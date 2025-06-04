#include "descriptor_sets.hpp"
#include "utility/exception.hpp"

namespace eru
{
   DescriptorSets::DescriptorSets(vk::raii::DescriptorPool pool, NamedSets sets)
      : pool_{ std::move(pool) }
      , sets_{ std::move(sets) }
   {
   }

   std::span<vk::raii::DescriptorSet const> DescriptorSets::sets(std::string_view const name) const
   {
      auto const set{ sets_.find(name.data()) };
      if (set == sets_.end())
         exception("descriptor set with name \"{}\" does not exist!", name);

      return std::get<Sets>(set->second);
   }

   DescriptorSets::Layout const& DescriptorSets::layout(std::string_view name) const
   {
      auto const set{ sets_.find(name.data()) };
      if (set == sets_.end())
         exception("descriptor set with name \"{}\" does not exist!", name);

      return std::get<Layout>(set->second);
   }

   std::uint32_t DescriptorSets::binding(std::string_view set_name, std::string_view binding_name) const
   {
      auto const set{ sets_.find(set_name.data()) };
      if (set == sets_.end())
         exception("descriptor set with name \"{}\" does not exist!", set_name);

      auto const& binding_map{ std::get<BindingMap>(set->second) };
      auto const binding{ binding_map.find(binding_name.data()) };
      if (binding == binding_map.end())
         exception("descriptor set \"{}\" does not have a binding with name \"{}\"!", set_name, binding_name);

      return binding->second;
   }
}