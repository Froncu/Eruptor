#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"
#include "eruptor/renderer_context.hpp"
#include "eruptor/runtime_assert.hpp"
#include "eruptor/window.hpp"

namespace eru
{
   VKAPI_ATTR auto VKAPI_CALL debug_callback(vk::DebugUtilsMessageSeverityFlagBitsEXT const severity, vk::DebugUtilsMessageTypeFlagsEXT const,
      vk::DebugUtilsMessengerCallbackDataEXT const* const callback_data, void* const) -> vk::Bool32
   {
      switch (severity)
      {
         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            [[fallthrough]];

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            Locator::get<Logger>().info(callback_data->pMessage);
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            Locator::get<Logger>().warning(callback_data->pMessage);
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            Locator::get<Logger>().error(callback_data->pMessage);
            break;

         default:
            break;
      }

      return vk::False;
   }

   RendererContext::RendererContext(std::initializer_list<char const* const> const requested_instance_extension_names,
      bool const create_messenger)
      : instance_{
         [this, &requested_instance_extension_names, create_messenger] -> vk::raii::Instance
         {
            vk::ApplicationInfo constexpr app_info{
               .apiVersion{ vk::HeaderVersionComplete }
            };

            std::vector<char const*> instance_extension_names{};
            for (std::string_view const required_instance_extension_name : Window::required_instance_extension_names())
               instance_extension_names.emplace_back(required_instance_extension_name.data());
            for (std::string_view const requested_instance_extension_name : requested_instance_extension_names)
               instance_extension_names.emplace_back(requested_instance_extension_name.data());
            if (create_messenger)
               instance_extension_names.emplace_back(vk::EXTDebugUtilsExtensionName);

            std::ranges::sort(instance_extension_names);
            auto const erased_range{ std::ranges::unique(instance_extension_names) };
            instance_extension_names.erase(std::ranges::begin(erased_range), std::ranges::end(erased_range));

            vk::ResultValue instance{
               context_.createInstance({
                  .pApplicationInfo{ &app_info },
                  .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(instance_extension_names)) },
                  .ppEnabledExtensionNames{ std::ranges::data(instance_extension_names) }
               })
            };
            RUNTIME_ASSERT(instance.has_value(),
               std::format("failed to create a Vulkan instance! ({})", to_string(instance.result)));

            return std::move(*instance);
         }()
      }
      , debug_messenger_{
         [this, create_messenger] -> std::optional<vk::raii::DebugUtilsMessengerEXT>
         {
            if (not create_messenger)
               return std::nullopt;

            vk::ResultValue debug_messenger{
               instance_.createDebugUtilsMessengerEXT({
                  .messageSeverity{
                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                  },
                  .messageType{
                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
                  },
                  .pfnUserCallback{ &debug_callback }
               })
            };
            RUNTIME_ASSERT(debug_messenger.has_value(),
               std::format("failed to create a debug messenger! ({})", to_string(debug_messenger.result)));

            return std::move(*debug_messenger);
         }()
      }
   {
   }

   auto RendererContext::instance() const -> vk::raii::Instance const&
   {
      return instance_;
   }
}