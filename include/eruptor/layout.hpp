#ifndef LAYOUT_HPP
#define LAYOUT_HPP

#include "eruptor/locator.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Context;

   class Layout final
   {
      public:
         struct Binding final
         {
            std::string name;
            vk::DescriptorType type;
            std::uint32_t capacity;
            std::string_view element_type;
         };

         struct Set final
         {
            std::uint32_t capacity;
            std::initializer_list<Binding> bindings;
         };

         ERU_API explicit Layout(std::string_view name, std::initializer_list<Set const> const& sets);
         Layout(Layout const&) = delete;
         Layout(Layout&&) = default;

         ~Layout() = default;

         auto operator=(Layout const&) -> Layout& = delete;
         auto operator=(Layout&&) -> Layout& = delete;

      private:
         Context const& context_{ Locator::get<Context>() };

         std::filesystem::path const shader_path_;

         std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts_{};
         vk::raii::PipelineLayout pipeline_layout_{ nullptr };
         vk::raii::DescriptorPool descriptor_pool_{ nullptr };
   };
}

#endif