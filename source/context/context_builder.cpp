#include "context_builder.hpp"

namespace eru
{
   ContextBuilder& ContextBuilder::enable_validation_layer(std::string validation_layer_name)
   {
      validation_layer_names_.insert(std::move(validation_layer_name));
      return *this;
   }

   ContextBuilder& ContextBuilder::enable_validation_layers(std::span<std::string> const validation_layer_names)
   {
      validation_layer_names_.insert(validation_layer_names.begin(), validation_layer_names.end());
      return *this;
   }

   ContextBuilder& ContextBuilder::enable_extension(std::string extenion_name)
   {
      extension_names_.insert(std::move(extenion_name));
      return *this;
   }

   ContextBuilder& ContextBuilder::enable_extensions(std::span<std::string> const extenion_names)
   {
      extension_names_.insert(extenion_names.begin(), extenion_names.end());
      return *this;
   }

   Context ContextBuilder::build()
   {
      std::uint32_t sdl_extension_count;
      char const* const* const sdl_extension_names{ SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count) };

      extension_names_.insert(sdl_extension_names, sdl_extension_names + sdl_extension_count);

      if (not validation_layer_names_.empty())
         extension_names_.insert(vk::EXTDebugUtilsExtensionName);

      auto validation_layer_names_view{
         std::views::transform(validation_layer_names_,
            [](std::string const& validation_layer_name) -> char const*
            {
               return validation_layer_name.c_str();
            })
      };
      std::vector<char const*> validation_layer_names{ validation_layer_names_view.begin(), validation_layer_names_view.end() };

      auto extension_names_view{
         std::views::transform(extension_names_,
            [](std::string const& extension_name) -> char const*
            {
               return extension_name.c_str();
            })
      };
      std::vector<char const*> extension_names{ extension_names_view.begin(), extension_names_view.end() };

      vk::raii::Context context{};
      vk::raii::Instance instance{
         context, {
            .enabledLayerCount{ static_cast<std::uint32_t>(validation_layer_names.size()) },
            .ppEnabledLayerNames{ validation_layer_names.data() },
            .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
            .ppEnabledExtensionNames{ extension_names.data() }
         }
      };

      auto debug_messenger{
         validation_layer_names_.empty()
            ? nullptr
            : vk::raii::DebugUtilsMessengerEXT{
               instance, {
                  .messageSeverity{
                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                  },
                  .messageType{
                     vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                  },
                  .pfnUserCallback{
                     [](VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                     VkDebugUtilsMessengerCallbackDataEXT const* callback_data, void*) -> VkBool32
                     {
                        std::cout << std::format("[VALIDATION LAYER MESSAGE]\n{}\n\n", callback_data->pMessage);
                        return false;
                     }
                  }
               }
            }
      };

      return { std::move(context), std::move(instance), std::move(debug_messenger) };
   }
}