#include "eruptor/context.hpp"
#include "eruptor/layout.hpp"
#include "eruptor/runtime_assert.hpp"

namespace
{
   [[nodiscard]] auto declared_type(eru::Layout::Binding const& binding) -> std::string
   {
      auto const element_type{
         [&binding](std::string_view const fallback) -> std::string_view
         {
            return binding.element_type.empty() ? fallback : binding.element_type;
         }
      };

      switch (binding.type)
      {
         case vk::DescriptorType::eSampler:
            return "SamplerState";

         case vk::DescriptorType::eCombinedImageSampler:
            return "Sampler2D";

         case vk::DescriptorType::eSampledImage:
            return "Texture2D";

         case vk::DescriptorType::eStorageImage:
            return std::format("RWTexture2D<{}>", element_type("float4"));

         case vk::DescriptorType::eUniformTexelBuffer:
            return std::format("Buffer<{}>", element_type("float4"));

         case vk::DescriptorType::eStorageTexelBuffer:
            return std::format("RWBuffer<{}>", element_type("float4"));

         case vk::DescriptorType::eUniformBuffer:
         case vk::DescriptorType::eUniformBufferDynamic:
            return std::format("ConstantBuffer<{}, ScalarDataLayout>", element_type({}));

         case vk::DescriptorType::eStorageBuffer:
         case vk::DescriptorType::eStorageBufferDynamic:
            return std::format("StructuredBuffer<{}, ScalarDataLayout>", element_type({}));

         case vk::DescriptorType::eInputAttachment:
            return "SubpassInput";

         case vk::DescriptorType::eAccelerationStructureKHR:
            return "RaytracingAccelerationStructure";

         default:
            return {};
      }
   }

   [[nodiscard]] auto needs_element_type(vk::DescriptorType const type) -> bool
   {
      switch (type)
      {
         case vk::DescriptorType::eUniformBuffer:
         case vk::DescriptorType::eUniformBufferDynamic:
         case vk::DescriptorType::eStorageBuffer:
         case vk::DescriptorType::eStorageBufferDynamic:
            return true;

         default:
            return false;
      }
   }
}

namespace eru
{
   Layout::Layout(std::string_view const name, std::initializer_list<Set const> const& sets)
      : shader_path_{
         [name, sets]
         {
            std::string const macro{
               std::format("ERU_{:s}_LAYOUT", name | std::views::transform(
                  [](unsigned char const character)
                  {
                     return static_cast<char>(std::toupper(character));
                  }))
            };

            std::string shader{
               "// Auto-generated from eru::Layout - do not edit"
               "\n"
               "\n#ifndef " + macro +
               "\n#define " + macro +
               "\n"
            };

            for (auto const& [set_index, set] : std::views::enumerate(sets))
            {
               for (auto const& [binding_index, binding] : std::views::enumerate(set.bindings))
               {
                  RUNTIME_ASSERT(not binding.name.empty(),
                     std::format("binding {} of set {} has no name to declare it under!", binding_index, set_index));

                  RUNTIME_ASSERT(not needs_element_type(binding.type) or not binding.element_type.empty(),
                     std::format("binding \"{}\" is a buffer and needs an element type to be declarable in Slang!", binding.name));

                  std::string const type{ declared_type(binding) };
                  RUNTIME_ASSERT(not type.empty(),
                     std::format("binding \"{}\" has a descriptor type with no Slang equivalent! ({})", binding.name, to_string(binding.type)));

                  shader += std::format("\n[[vk::binding({}, {})]] {} {}{};",
                     binding_index, set_index, type, binding.name,
                     binding.capacity > 1 ? std::format("[{}]", binding.capacity) : std::string{});
               }

               shader += "\n";
            }

            shader += "\n#endif";

            std::filesystem::path const shader_path{ std::format("{}/generated/shaders/layouts/{}_layout.slang", COMPILE_SOURCE_PATH, name) };
            create_directories(shader_path.parent_path());
            if (std::ofstream stream{ shader_path };
               stream.is_open())
               stream << shader;

            return shader_path;
         }()
      }
   {
      std::vector<vk::DescriptorPoolSize> desciptor_pool_sizes{};
      std::vector<vk::DescriptorSetLayout> raw_descriptor_set_layouts{};
      std::uint32_t max_sets{};

      descriptor_set_layouts_.reserve(std::ranges::size(sets));
      for (auto const& [set_index, set] : std::views::enumerate(sets))
      {
         std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings{};
         descriptor_set_layout_bindings.reserve(set.bindings.size());
         for (auto const& [binding_index, binding] : std::views::enumerate(set.bindings))
         {
            descriptor_set_layout_bindings.push_back({
               .binding{ static_cast<std::uint32_t>(binding_index) },
               .descriptorType{ binding.type },
               .descriptorCount{ binding.capacity },
               .stageFlags{ vk::ShaderStageFlagBits::eAll }
            });

            if (auto const descriptor_pool_size{ std::ranges::find(desciptor_pool_sizes, binding.type, &vk::DescriptorPoolSize::type) };
               descriptor_pool_size == std::ranges::end(desciptor_pool_sizes))
               desciptor_pool_sizes.push_back({
                  .type{ binding.type },
                  .descriptorCount{ set.capacity * binding.capacity }
               });
            else
               descriptor_pool_size->descriptorCount += set.capacity * binding.capacity;
         }

         vk::ResultValue descriptor_set_layout{
            context_.device.createDescriptorSetLayout({
               .bindingCount{ static_cast<std::uint32_t>(std::ranges::size(descriptor_set_layout_bindings)) },
               .pBindings{ std::ranges::data(descriptor_set_layout_bindings) }
            })
         };
         RUNTIME_ASSERT(descriptor_set_layout.has_value(),
            std::format("failed to create descriptor set layout! ({})", to_string(descriptor_set_layout.result)));

         raw_descriptor_set_layouts.push_back(descriptor_set_layouts_.emplace_back(std::move(descriptor_set_layout.value)));
         max_sets += set.capacity;
      }

      //====//

      vk::ResultValue pipeline_layout{
         context_.device.createPipelineLayout({
            .setLayoutCount{ static_cast<std::uint32_t>(std::ranges::size(raw_descriptor_set_layouts)) },
            .pSetLayouts{ std::ranges::data(raw_descriptor_set_layouts) }
         })
      };
      RUNTIME_ASSERT(pipeline_layout.has_value(),
         std::format("failed to create a pipeline layout! ({})", to_string(pipeline_layout.result)));

      pipeline_layout_ = std::move(*pipeline_layout);

      //====//

      vk::ResultValue descriptor_pool{
         context_.device.createDescriptorPool({
            .maxSets{ max_sets },
            .poolSizeCount{ static_cast<std::uint32_t>(std::ranges::size(desciptor_pool_sizes)) },
            .pPoolSizes{ std::ranges::data(desciptor_pool_sizes) }
         })
      };
      RUNTIME_ASSERT(descriptor_pool.has_value(),
         std::format("failed to create a descriptor pool! ({})", to_string(descriptor_pool.result)));

      descriptor_pool_ = std::move(*descriptor_pool);
   }
}