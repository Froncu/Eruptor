#include "descriptor_sets_builder.hpp"
#include "utility/exception.hpp"

namespace eru
{
   DescriptorSetsBuilder& DescriptorSetsBuilder::add_descriptor_set(DescriptorSetInfo descriptor_set_info)
   {
      if (descriptor_set_info.name.empty())
         exception("the descriptor set name cannot be empty!");

      if (not descriptor_set_info.bindings.size())
         exception("the descriptor set's \"{}\" bindings cannot be empty!", descriptor_set_info.name);

      if (std::ranges::any_of(descriptor_set_info.bindings,
         [](DescriptorBindingInfo const& binding)
         {
            return
               binding.name.empty() or
               binding.count == 0;
         }))
         exception("at least one of the descriptor set's \"{}\" bindings is unnamed or has a count of 0!",
            descriptor_set_info.name);

      if (std::unordered_set<std::string> names{}; std::ranges::any_of(descriptor_set_info.bindings,
         [&names](DescriptorBindingInfo const& binding)
         {
            return
               binding.name.empty() or
               not names.insert(binding.name).second;
         }))
         exception("all binding names in the descriptor set \"{}\" must be unique and not empty!", descriptor_set_info.name);

      if (not descriptor_set_info.allocation_count)
         exception("the descriptor set's \"{}\" allocation count cannot be 0!", descriptor_set_info.name);

      if (auto stored_set{ descriptor_sets_.find(descriptor_set_info.name) };
         stored_set == descriptor_sets_.end())
      {
         for (stored_set = descriptor_sets_.begin(); stored_set not_eq descriptor_sets_.end(); ++stored_set)
         {
            if (stored_set->bindings == descriptor_set_info.bindings)
               exception("an already added descriptor set named \"{}\" has identical bindings to \"{}\"!",
                  stored_set->name, descriptor_set_info.name);
         }

         descriptor_sets_.emplace(std::move(descriptor_set_info));
      }
      else
         exception("a descriptor set with name \"{}\" already exists!",
            stored_set->name);

      return *this;
   }

   DescriptorSetsBuilder& DescriptorSetsBuilder::add_descriptor_sets(
      std::initializer_list<DescriptorSetInfo> const descriptor_set_infos)
   {
      for (DescriptorSetInfo const& descriptor_set : descriptor_set_infos)
         add_descriptor_set(descriptor_set);

      return *this;
   }

   DescriptorSets DescriptorSetsBuilder::build(Device const& device) const
   {
      DescriptorSets::NamedSets named_sets{};
      named_sets.reserve(descriptor_sets_.size());
      create_descriptor_set_layouts(device, named_sets);

      vk::raii::DescriptorPool pool{ create_descriptor_pool(device) };
      allocate_descriptor_sets(device, pool, named_sets);

      return { std::move(pool), std::move(named_sets) };
   }

   void DescriptorSetsBuilder::create_descriptor_set_layouts(Device const& device, DescriptorSets::NamedSets& named_sets) const
   {
      for (auto const& [set_name, bindings, allocation_count] : descriptor_sets_)
      {
         std::vector<vk::DescriptorSetLayoutBinding> native_bindings{};
         native_bindings.reserve(bindings.size());
         DescriptorSets::BindingMap binding_map{};
         binding_map.reserve(bindings.size());
         std::vector<vk::DescriptorBindingFlags> flags_collection{};
         flags_collection.reserve(bindings.size());
         for (std::uint32_t index{}; index < bindings.size(); ++index)
         {
            auto const& [binding_name, flags, type, shader_stage_flags, count]{ bindings[index] };
            native_bindings.push_back({
               .binding{ index },
               .descriptorType{ type },
               .descriptorCount{ count },
               .stageFlags{ shader_stage_flags }
            });

            binding_map.emplace(binding_name, index);

            flags_collection.emplace_back(flags);
         }

         std::uint32_t const binding_count{ static_cast<std::uint32_t>(bindings.size()) };
         vk::DescriptorSetLayoutBindingFlagsCreateInfo const binding_flags_create_info{
            .bindingCount{ binding_count },
            .pBindingFlags{ flags_collection.data() }
         };

         named_sets.emplace(
            set_name,
            std::tuple{
               DescriptorSets::Sets{},
               device.device().createDescriptorSetLayout({
                  .pNext{ &binding_flags_create_info },
                  .bindingCount{ binding_count },
                  .pBindings{ native_bindings.data() }
               }),
               std::move(binding_map)
            }
         );
      }
   }

   vk::raii::DescriptorPool DescriptorSetsBuilder::create_descriptor_pool(Device const& device) const
   {
      std::unordered_map<vk::DescriptorType, std::uint32_t> type_sizes{};
      std::uint32_t total_set_count{};
      for (auto const& [name, bindings, allocation_count] : descriptor_sets_)
      {
         for (DescriptorBindingInfo const binding : bindings)
            type_sizes[binding.type] += binding.count * allocation_count;

         total_set_count += allocation_count;
      }

      std::vector<vk::DescriptorPoolSize> pool_size{};
      for (auto const& [type, size] : type_sizes)
         pool_size.emplace_back(type, size);

      return device.device().createDescriptorPool({
         .flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet },
         .maxSets{ total_set_count },
         .poolSizeCount{ static_cast<std::uint32_t>(pool_size.size()) },
         .pPoolSizes{ pool_size.data() }
      });
   }

   void DescriptorSetsBuilder::allocate_descriptor_sets(Device const& device, vk::raii::DescriptorPool const& pool,
      DescriptorSets::NamedSets& named_sets) const
   {
      for (auto& [name, set] : named_sets)
      {
         std::vector<vk::DescriptorSetLayout> layouts{
            descriptor_sets_.find(name)->allocation_count, std::get<vk::raii::DescriptorSetLayout>(set)
         };

         std::get<std::vector<vk::raii::DescriptorSet>>(set) =
            device.device().allocateDescriptorSets({
               .descriptorPool{ *pool },
               .descriptorSetCount{ static_cast<std::uint32_t>(layouts.size()) },
               .pSetLayouts{ layouts.data() }
            });
      }
   }
}