#ifndef DESCRIPTOR_SETS_BUILDER_HPP
#define DESCRIPTOR_SETS_BUILDER_HPP

#include "erupch/erupch.hpp"
#include "renderer/device.hpp"
#include "renderer/descriptor_sets.hpp"

namespace eru
{
   class DescriptorSetsBuilder final
   {
      public:
         struct DescriptorBindingInfo final
         {
            std::string name{};
            vk::DescriptorBindingFlags flags{};
            vk::DescriptorType type{ vk::DescriptorType::eUniformBuffer };
            vk::ShaderStageFlags shader_stage_flags{ vk::ShaderStageFlagBits::eFragment };
            std::uint32_t count{ 1 };

            [[nodiscard]] bool operator==(DescriptorBindingInfo const& other) const
            {
               return flags == other.flags and
                  type == other.type and
                  shader_stage_flags == other.shader_stage_flags and
                  count == other.count;
            };
         };

         struct DescriptorSetInfo final
         {
            std::string name{};
            std::vector<DescriptorBindingInfo> bindings{};
            std::uint32_t allocation_count{ 1 };

            struct Equal final
            {
               using is_transparent = void;

               [[nodiscard]] bool operator()(DescriptorSetInfo const& a, DescriptorSetInfo const& b) const
               {
                  return a.name == b.name;
               }

               [[nodiscard]] bool operator()(std::string_view const name, DescriptorSetInfo const& set) const
               {
                  return name == set.name;
               }

               [[nodiscard]] bool operator()(DescriptorSetInfo const& set, std::string_view const name) const
               {
                  return set.name == name;
               }
            };

            struct Hasher final
            {
               using is_transparent = void;

               [[nodiscard]] std::size_t operator()(DescriptorSetInfo const& set) const
               {
                  return std::hash<std::string>{}(set.name);
               }

               [[nodiscard]] std::size_t operator()(std::string_view const name) const
               {
                  return std::hash<std::string_view>{}(name);
               }

               [[nodiscard]] std::size_t operator()(std::string const& name) const
               {
                  return std::hash<std::string>{}(name);
               }
            };
         };

         DescriptorSetsBuilder() = default;
         DescriptorSetsBuilder(DescriptorSetsBuilder const&) = delete;
         DescriptorSetsBuilder(DescriptorSetsBuilder&&) = delete;

         ~DescriptorSetsBuilder() = default;

         DescriptorSetsBuilder& operator=(DescriptorSetsBuilder const&) = delete;
         DescriptorSetsBuilder& operator=(DescriptorSetsBuilder&&) = delete;

         DescriptorSetsBuilder& add_descriptor_set(DescriptorSetInfo descriptor_set_info);
         DescriptorSetsBuilder& add_descriptor_sets(std::initializer_list<DescriptorSetInfo> descriptor_set_infos);

         [[nodiscard]] DescriptorSets build(Device const& device) const;

      private:
         void create_descriptor_set_layouts(Device const& device, DescriptorSets::NamedSets& named_sets) const;
         [[nodiscard]] vk::raii::DescriptorPool create_descriptor_pool(Device const& device) const;
         void allocate_descriptor_sets(Device const& device, vk::raii::DescriptorPool const& pool,
            DescriptorSets::NamedSets& named_sets) const;

         std::unordered_set<DescriptorSetInfo, DescriptorSetInfo::Hasher, DescriptorSetInfo::Equal> descriptor_sets_{};
   };
}

#endif