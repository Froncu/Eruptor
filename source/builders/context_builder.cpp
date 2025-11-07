#include "context_builder.hpp"

#include "utility/exception.hpp"

namespace eru
{
   ContextBuilder& ContextBuilder::change_api_version(std::uint32_t const api_version)
   {
      switch (api_version)
      {
         case vk::ApiVersion10:
         case vk::ApiVersion11:
         case vk::ApiVersion12:
         case vk::ApiVersion13:
         case vk::ApiVersion14:
            api_version_ = api_version;
            break;

         default:
            exception("invalid Vulkan API version specified: {}", api_version);
      }

      return *this;
   }

   ContextBuilder& ContextBuilder::enable_validation_layer(std::string_view const validation_layer_name)
   {
      if (not validation_layer_name.empty())
         validation_layer_names_.insert(validation_layer_name.data());

      return *this;
   }

   ContextBuilder& ContextBuilder::enable_validation_layers(
      std::initializer_list<std::string_view> const validation_layer_names)
   {
      for (std::string_view const validation_layer_name : validation_layer_names)
         enable_validation_layer(validation_layer_name);

      return *this;
   }

   ContextBuilder& ContextBuilder::enable_instance_extension(std::string_view const extenion_name)
   {
      if (not extenion_name.empty())
         instance_extension_names_.insert(extenion_name.data());

      return *this;
   }

   ContextBuilder& ContextBuilder::enable_instance_extensions(std::initializer_list<std::string_view> const extenion_names)
   {
      for (std::string_view const extenion_name : extenion_names)
         enable_instance_extension(extenion_name);

      return *this;
   }

   Context ContextBuilder::build()
   {
      std::uint32_t sdl_extension_count;
      char const* const* const sdl_extension_names{ SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count) };

      instance_extension_names_.insert(sdl_extension_names, sdl_extension_names + sdl_extension_count);

      if (not validation_layer_names_.empty())
         instance_extension_names_.insert(vk::EXTDebugUtilsExtensionName);

      auto validation_layer_names_view{
         std::views::transform(validation_layer_names_,
            [](std::string const& validation_layer_name)
            {
               return validation_layer_name.c_str();
            })
      };
      std::vector<char const*> validation_layer_names{ validation_layer_names_view.begin(), validation_layer_names_view.end() };

      auto extension_names_view{
         std::views::transform(instance_extension_names_,
            [](std::string const& extension_name)
            {
               return extension_name.c_str();
            })
      };
      std::vector<char const*> extension_names{ extension_names_view.begin(), extension_names_view.end() };

      vk::ApplicationInfo const application_info{
         .apiVersion{ api_version_ },
      };

      vk::raii::Context context{};
      vk::raii::Instance instance{
         context, {
            .pApplicationInfo{ &application_info },
            .enabledLayerCount{ static_cast<std::uint32_t>(validation_layer_names.size()) },
            .ppEnabledLayerNames{ validation_layer_names.data() },
            .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
            .ppEnabledExtensionNames{ extension_names.data() }
         }
      };;

      auto debug_messenger{
         validation_layer_names_.empty()
            ? nullptr
            : instance.createDebugUtilsMessengerEXT({
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
                  [](vk::DebugUtilsMessageSeverityFlagBitsEXT, vk::DebugUtilsMessageTypeFlagsEXT,
                  vk::DebugUtilsMessengerCallbackDataEXT const* callback_data, void*) -> vk::Bool32
                  {
                     std::cout << std::format("[VALIDATION LAYER MESSAGE]\n{}\n\n", callback_data->pMessage);
                     return false;
                  }
               }
            })
      };

      return { std::move(context), std::move(instance), std::move(debug_messenger), api_version_ };
   }
}