#include "eruptor/application.hpp"
#include "eruptor/exception.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"
#include "eruptor/runtime_assert.hpp"

#include "core/dependencies.hpp"

namespace eru
{
   VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT const severity,
      vk::DebugUtilsMessageTypeFlagsEXT const type,
      vk::DebugUtilsMessengerCallbackDataEXT const* const callback_data,
      void* const)
   {
      std::string message{ std::format("{}\n{}", to_string(type), callback_data->pMessage) };
      switch (severity)
      {
         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            Locator::get<Logger>().info(std::move(message));
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            Locator::get<Logger>().warning(std::move(message));
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            Locator::get<Logger>().error(std::move(message));
            break;

         default:
            break;
      }

      return vk::False;
   }

   Application::LocatorRegistrator::LocatorRegistrator(Application& application)
   {
      Locator::provide<Application>(application);
      Locator::provide<Logger>();
   }

   Application::LocatorRegistrator::~LocatorRegistrator()
   {
      Locator::remove_all();
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

   void Application::poll()
   {
      glfwPollEvents();
   }

   Application::Application(std::string_view const name, std::uint32_t const version)
      : instance_{ instance(name, version) }
   {
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
      return {
         instance_,
         {
            .messageSeverity{
               vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
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
         }
      };
   }

   vk::raii::SurfaceKHR Application::surface() const
   {
      VkSurfaceKHR surface;
      glfwCreateWindowSurface(*instance_, &window_.native(), nullptr, &surface);
      return { instance_, surface };
   }

   vk::raii::PhysicalDevice Application::physical_device() const
   {
      std::vector const physical_devices{ instance_.enumeratePhysicalDevices() };
      auto const physical_device{
         std::ranges::find_if(
            physical_devices,
            [](vk::raii::PhysicalDevice const& device)
            {
               vk::PhysicalDeviceProperties const properties{ device.getProperties() };
               vk::PhysicalDeviceFeatures const features{ device.getFeatures() };

               return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu
                  and features.wideLines;
            })
      };

      if (physical_device == std::ranges::end(physical_devices))
         throw Exception{ "no suitable physical device found!" };

      return *physical_device;
   }

   std::uint32_t Application::queue_family_index() const
   {
      auto&& queue_family_properties{ physical_device_.getQueueFamilyProperties2() | std::ranges::views::enumerate };
      auto const queue_family{
         std::ranges::find_if(
            queue_family_properties,
            [this](auto&& pair)
            {
               auto&& [index, properties]{ pair };
               return static_cast<bool>(properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
                  and physical_device_.getSurfaceSupportKHR(index, surface_);
            })
      };

      if (queue_family == std::ranges::end(queue_family_properties))
         throw Exception{ "no suitable queue family found!" };

      return static_cast<std::uint32_t>(queue_family.index());
   }

   vk::raii::Device Application::device() const
   {
      vk::StructureChain<
         vk::PhysicalDeviceFeatures2,
         vk::PhysicalDeviceVulkan13Features,
         vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> const device_feature_chain{
         {
         },
         {
            .dynamicRendering{ true }
         },
         {
            .extendedDynamicState{ true }
         }
      };

      auto constexpr queue_priority{ 0.5f };

      std::array<vk::DeviceQueueCreateInfo, 1> const device_queue_create_info{
         {
            {
               .flags{},
               .queueFamilyIndex{ queue_family_index_ },
               .queueCount{ 1 },
               .pQueuePriorities{ &queue_priority }
            },
         }
      };

      std::array<char const*, 1> constexpr device_extension_names{
         {
            vk::KHRSwapchainExtensionName
         }
      };

      // TODO: for backwards compatibility, the validation layers here should be the same as the ones enabled on the instance
      return
      {
         physical_device_,
         {
            .pNext{ device_feature_chain.get() },
            .queueCreateInfoCount{ static_cast<std::uint32_t>(device_queue_create_info.size()) },
            .pQueueCreateInfos{ device_queue_create_info.data() },
            .enabledLayerCount{},
            .ppEnabledLayerNames{},
            .enabledExtensionCount{ static_cast<std::uint32_t>(device_extension_names.size()) },
            .ppEnabledExtensionNames{ device_extension_names.data() },
         }
      };
   }

   vk::raii::Queue Application::queue() const
   {
      return { device_, queue_family_index_, 0 };
   }
}