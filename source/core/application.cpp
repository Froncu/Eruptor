#include "eruptor/application.hpp"
#include "eruptor/exception.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"
#include "eruptor/runtime_assert.hpp"
#include "eruptor/window.hpp"

#include "core/dependencies.hpp"

namespace eru
{
   VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT const severity,
      vk::DebugUtilsMessageTypeFlagsEXT const type,
      vk::DebugUtilsMessengerCallbackDataEXT const* const callback_data,
      void* const)
   {
      std::string message{ std::format("{}\n{}", vk::to_string(type), callback_data->pMessage) };
      switch (severity)
      {
         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            [[fallthrough]];

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            Locator::get<Logger>().info(std::move(message));
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            Locator::get<Logger>().warning(std::move(message));
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            Locator::get<Logger>().error(std::move(message));
            break;
      }

      return vk::False;
   }

   Application::GLFWcontext::GLFWcontext()
   {
      glfwSetErrorCallback(
         [](int const code, char const* const description)
         {
            runtime_assert(false, std::format("GLFW encountered error code {}! ({})", code, description));
         });

      glfwInit();
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
   }

   Application::GLFWcontext::~GLFWcontext()
   {
      glfwTerminate();
   }

   Application::~Application()
   {
      Locator::remove_all();
   }

   void Application::poll()
   {
      glfwPollEvents();
   }

   Application::Application(std::string_view const name, std::uint32_t const version)
      : instance_{ instance(name, version) }
   {
      Locator::provide<Application>(*this);
      Locator::provide<Logger>();
   }

   vk::raii::Instance Application::instance(std::string_view const name, std::uint32_t const version) const
   {
      vk::ApplicationInfo const app_info{
         .pApplicationName{ name.data() },
         .applicationVersion{ version },
         .pEngineName{ "eruptor" },
         .engineVersion{ VK_MAKE_VERSION(0, 0, 0) },
         .apiVersion{ vk::HeaderVersionComplete }
      };

      std::vector<char const*> extension_names;

      {
         std::uint32_t required_instance_extensions_count;
         char const** const required_instance_extensions{
            glfwGetRequiredInstanceExtensions(&required_instance_extensions_count)
         };

         extension_names.assign(required_instance_extensions,
            required_instance_extensions + required_instance_extensions_count);
      }

      extension_names.push_back(vk::EXTDebugUtilsExtensionName);

      std::array<char const*, 1> constexpr layer_names{ "VK_LAYER_KHRONOS_validation" };

      return {
         vulkan_context_, {
            .flags{},
            .pApplicationInfo{ &app_info },
            .enabledLayerCount{ static_cast<std::uint32_t>(layer_names.size()) },
            .ppEnabledLayerNames{ layer_names.data() },
            .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
            .ppEnabledExtensionNames{ extension_names.data() }
         }
      };
   }

   vk::raii::DebugUtilsMessengerEXT Application::debug_messenger() const
   {
      return instance_.createDebugUtilsMessengerEXT({
         .messageSeverity{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
         },
         .messageType{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
         },
         .pfnUserCallback{ &debug_callback }
      });
   }

   vk::raii::PhysicalDevice Application::physical_device() const
   {
      for (vk::raii::PhysicalDevice const& device : instance_.enumeratePhysicalDevices())
      {
         vk::PhysicalDeviceProperties const properties{ device.getProperties() };
         vk::PhysicalDeviceFeatures const features{ device.getFeatures() };
         if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu and features.wideLines)
            return device;
      }

      throw Exception{ "no suitable device found!" };
   }
}