#include "eruptor/context.hpp"
#include "eruptor/runtime_assert.hpp"
#include "eruptor/window.hpp"

namespace
{
   VKAPI_ATTR auto VKAPI_CALL debug_callback(
      vk::DebugUtilsMessageSeverityFlagBitsEXT const severity,
      vk::DebugUtilsMessageTypeFlagsEXT const,
      vk::DebugUtilsMessengerCallbackDataEXT const* const callback_data,
      void* const) -> vk::Bool32
   {
      switch (severity)
      {
         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            [[fallthrough]];

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            eru::Locator::get<eru::Logger>().info(callback_data->pMessage);
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            eru::Locator::get<eru::Logger>().warning(callback_data->pMessage);
            break;

         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            eru::Locator::get<eru::Logger>().error(callback_data->pMessage);
            break;

         default:
            break;
      }

      return vk::False;
   }
}

namespace eru
{
   Context::Context(PassKey<Locator>)
   {
   }

   auto Context::allocate_memory(vk::MemoryRequirements const& requirements, vk::MemoryPropertyFlags const properties) const -> vk::raii::DeviceMemory
   {
      vk::PhysicalDeviceMemoryProperties2 const available_properties{ physical_device.getMemoryProperties2() };
      std::bitset<sizeof(requirements.memoryTypeBits) * 8> const type_bits{ requirements.memoryTypeBits };

      std::uint32_t index{};
      for (; index < available_properties.memoryProperties.memoryTypeCount; ++index)
         if (type_bits[index] and (available_properties.memoryProperties.memoryTypes[index].propertyFlags & properties) == properties)
            break;

      vk::MemoryPriorityAllocateInfoEXT constexpr priority_allocate_info{
         .priority{ 1.0f }
      };

      vk::ResultValue device_memory{
         device.allocateMemory({
            .pNext{ &priority_allocate_info },
            .allocationSize{ requirements.size },
            .memoryTypeIndex{ index }
         })
      };
      RUNTIME_ASSERT(device_memory.has_value(),
         std::format("failed to allocate memory! ({})", to_string(device_memory.result)));

      return std::move(*device_memory);
   }

   auto Context::create_semaphores(std::uint32_t const count) const -> std::vector<vk::raii::Semaphore>
   {
      std::vector<vk::raii::Semaphore> semaphores{};
      semaphores.reserve(count);
      for (std::size_t index{}; index < count; ++index)
      {
         vk::ResultValue semaphore{ device.createSemaphore({}) };
         RUNTIME_ASSERT(semaphore.has_value(),
            std::format("failed create a semaphore! ({})", to_string(semaphore.result)));

         semaphores.push_back(std::move(*semaphore));
      }

      return semaphores;
   }

   auto Context::create_instance() const -> vk::raii::Instance
   {
      vk::ApplicationInfo constexpr app_info{
         // .pApplicationName{},
         // .applicationVersion{},
         .pEngineName{ "eruptor" },
         .engineVersion{ VK_MAKE_VERSION(0, 0, 0) },
         .apiVersion{ vk::HeaderVersionComplete }
      };

      std::vector<char const*> extension_names{};
      for (std::string_view const required_extension_names : Window::required_instance_extension_names())
         extension_names.emplace_back(required_extension_names.data());

      extension_names.push_back(vk::EXTDebugUtilsExtensionName);

      vk::ResultValue instance{
         vulkan_context.createInstance({
            .flags{},
            .pApplicationInfo{ &app_info },
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(extension_names) }
         })
      };
      RUNTIME_ASSERT(instance.has_value(),
         std::format("failed to create a Vulkan instance! ({})", to_string(instance.result)));

      return std::move(*instance);
   }

   auto Context::create_debug_messenger() const -> vk::raii::DebugUtilsMessengerEXT
   {
      vk::ResultValue debug_messenger{
         instance.createDebugUtilsMessengerEXT({
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
   }

   auto Context::pick_physical_device() const -> vk::raii::PhysicalDevice
   {
      vk::ResultValue const physical_devices{ instance.enumeratePhysicalDevices() };
      RUNTIME_ASSERT(physical_devices.has_value(),
         std::format("failed to query available physical devices! ({})", to_string(physical_devices.result)));

      auto const physical_device{
         std::ranges::find_if(
            *physical_devices,
            [](vk::raii::PhysicalDevice const& device)
            {
               vk::StructureChain const properties{ device.getProperties2() };
               vk::StructureChain const features{ device.getFeatures2() };

               return properties.get().properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu
                  and features.get().features.wideLines;
            })
      };

      RUNTIME_ASSERT(physical_device not_eq std::ranges::end(*physical_devices),
         "no suitable physical device found!");

      return *physical_device;
   }

   auto Context::pick_queue_family_index() const -> std::uint32_t
   {
      // TODO: use vk::StructureChain
      auto&& queue_family_properties{ physical_device.getQueueFamilyProperties2() | std::ranges::views::enumerate };
      auto const queue_family{
         std::ranges::find_if(
            queue_family_properties,
            [this](auto&& pair)
            {
               auto&& [index, properties]{ pair };
               return Window::presentation_support(instance, physical_device, static_cast<std::uint32_t>(index))
                  and static_cast<bool>(properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
            })
      };

      RUNTIME_ASSERT(queue_family not_eq std::ranges::end(queue_family_properties),
         "no suitable queue family found!");

      return static_cast<std::uint32_t>(queue_family.index());
   }

   auto Context::create_device() const -> vk::raii::Device
   {
      vk::StructureChain<
         vk::PhysicalDeviceFeatures2,
         vk::PhysicalDeviceVulkan11Features,
         vk::PhysicalDeviceVulkan13Features,
         vk::PhysicalDeviceVulkan14Features,
         vk::PhysicalDeviceSwapchainMaintenance1FeaturesEXT,
         vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT> const device_feature_chain{
         {
            .features
            {
               .samplerAnisotropy{ vk::True }
            }
         },
         {
            .shaderDrawParameters{ vk::True },
         },
         {
            .synchronization2{ vk::True },
            .dynamicRendering{ vk::True },
         },
         {
            .maintenance5{ vk::True }
         },
         {
            .swapchainMaintenance1{ vk::True }
         },
         {
            .pageableDeviceLocalMemory{ vk::True }
         }
      };

      auto constexpr queue_priority{ 0.5f };

      std::array const device_queue_create_info{
         std::to_array<vk::DeviceQueueCreateInfo>({
            {
               .flags{},
               .queueFamilyIndex{ queue_family_index },
               .queueCount{ 1 },
               .pQueuePriorities{ &queue_priority }
            }
         })
      };

      std::array constexpr device_extension_names{
         std::to_array<char const* const>({
            vk::EXTMemoryPriorityExtensionName,
            vk::EXTPageableDeviceLocalMemoryExtensionName,
            vk::KHRSwapchainExtensionName
         })
      };

      // TODO: for backwards compatibility, the validation layers here should be the same as the ones enabled on the instance
      vk::ResultValue device{
         physical_device.createDevice({
            .pNext{ device_feature_chain.get() },
            .queueCreateInfoCount{ static_cast<std::uint32_t>(std::ranges::size(device_queue_create_info)) },
            .pQueueCreateInfos{ std::ranges::data(device_queue_create_info) },
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(device_extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(device_extension_names) },
         })
      };
      RUNTIME_ASSERT(device.has_value(),
         std::format("failed to create a device! ({})", to_string(device.result)));

      return std::move(*device);
   }

   auto Context::retrieve_queue() const -> vk::raii::Queue
   {
      return device.getQueue2({
         .queueFamilyIndex{ queue_family_index },
         .queueIndex{ 0 }
      });
   }

   auto Context::create_command_pool() const -> vk::raii::CommandPool
   {
      vk::ResultValue command_pool{
         device.createCommandPool({
            .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
            .queueFamilyIndex{ queue_family_index }
         })
      };
      RUNTIME_ASSERT(command_pool.has_value(),
         std::format("failed create a command pool! ({})", to_string(command_pool.result)));

      return std::move(*command_pool);
   }
}