#ifndef LAYOUT_HPP
#define LAYOUT_HPP

#include "eruptor/locator.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Context;

   class Layout final
   {
      struct Description final
      {
         std::span<std::span<vk::DescriptorSetLayoutBinding const>> const sets;
         std::span<vk::PushConstantRange const> const push_constants;
      };

      public:
         ERU_API explicit Layout(Description const& description);

         Layout(Layout const&) = delete;
         Layout(Layout&&) = default;

         ~Layout() = default;

         auto operator=(Layout const&) -> Layout& = delete;
         auto operator=(Layout&&) -> Layout& = delete;

      private:
         std::vector<vk::raii::DescriptorSetLayout> descriptor_set_layouts_{};
         vk::raii::PipelineLayout pipeline_layout_{ nullptr };
   };
}

#endif