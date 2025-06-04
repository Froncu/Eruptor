#ifndef DESCRIPTOR_SETS_HPP
#define DESCRIPTOR_SETS_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   class DescriptorSets final
   {
      friend class DescriptorSetsBuilder;

      using Sets = std::vector<vk::raii::DescriptorSet>;
      using Layout = vk::raii::DescriptorSetLayout;
      using BindingMap = std::unordered_map<std::string, std::uint32_t>;
      using NamedSets = std::unordered_map<std::string, std::tuple<Sets, Layout, BindingMap>>;

      public:
         DescriptorSets(DescriptorSets const&) = delete;
         DescriptorSets(DescriptorSets&&) = default;

         ~DescriptorSets() = default;

         DescriptorSets& operator=(DescriptorSets const&) = delete;
         DescriptorSets& operator=(DescriptorSets&&) = default;

         [[nodiscard]] std::span<vk::raii::DescriptorSet const> sets(std::string_view name) const;
         [[nodiscard]] Layout const& layout(std::string_view name) const;
         [[nodiscard]] std::uint32_t binding(std::string_view set_name, std::string_view binding_name) const;

      private:
         DescriptorSets(vk::raii::DescriptorPool pool, NamedSets sets);

         vk::raii::DescriptorPool pool_;
         NamedSets sets_;
   };
}

#endif