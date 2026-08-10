#include "eruptor/context.hpp"
#include "eruptor/layout.hpp"
#include "eruptor/runtime_assert.hpp"

namespace eru
{
   Layout::Layout(Description const& description)
   {
      static Context const& CONTEXT{ Locator::get<Context>() };

      //====//

      std::vector<vk::DescriptorSetLayout> raw_descriptor_set_layouts{};
      descriptor_set_layouts_.reserve(std::ranges::size(description.sets));
      for (std::span const set : description.sets)
      {
         vk::ResultValue descriptor_set_layout{
            CONTEXT.device.createDescriptorSetLayout({
               .flags{  },
               .bindingCount{ static_cast<std::uint32_t>(std::ranges::size(set)) },
               .pBindings{ std::ranges::data(set) }
            })
         };
         RUNTIME_ASSERT(descriptor_set_layout.has_value(),
            std::format("failed to create descriptor set layout! ({})", to_string(descriptor_set_layout.result)));

         raw_descriptor_set_layouts.push_back(descriptor_set_layouts_.emplace_back(std::move(descriptor_set_layout.value)));
      }

      //====//

      vk::ResultValue pipeline_layout{
         CONTEXT.device.createPipelineLayout({
            .setLayoutCount{ static_cast<std::uint32_t>(std::ranges::size(raw_descriptor_set_layouts)) },
            .pSetLayouts{ std::ranges::data(raw_descriptor_set_layouts) },
            .pushConstantRangeCount{ static_cast<std::uint32_t>(std::ranges::size(description.push_constants)) },
            .pPushConstantRanges{ std::ranges::data(description.push_constants) }
         })
      };
      RUNTIME_ASSERT(pipeline_layout.has_value(),
         std::format("failed to create a pipeline layout! ({})", to_string(pipeline_layout.result)));

      pipeline_layout_ = std::move(*pipeline_layout);
   }
}
