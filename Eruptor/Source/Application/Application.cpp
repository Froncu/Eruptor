#include "erupch.hpp"

#include "application.hpp"

namespace eru
{
   application::application()
   {
   }

   application::~application()
   {
      device_.destroy();
      instance_.destroySurfaceKHR(surface_);
      instance_.destroy();
   }

   void application::run() const
   {
      while (not glfwWindowShouldClose(window_.get()))
         glfwPollEvents();
   }

   vk::Instance application::create_instance() const
   {
#if defined NDEBUG
      std::array<char const*, 0> constexpr valdiation_layer_names{};
#else
      std::array constexpr valdiation_layer_names{ "VK_LAYER_KHRONOS_validation" };
#endif

      std::uint32_t extension_count;
      char const* const* const extension_names{ glfwGetRequiredInstanceExtensions(&extension_count) };

      vk::InstanceCreateInfo const instance_info{
         .enabledLayerCount{ static_cast<std::uint32_t>(valdiation_layer_names.size()) },
         .ppEnabledLayerNames{ valdiation_layer_names.data() },
         .enabledExtensionCount{ extension_count },
         .ppEnabledExtensionNames{ extension_names }
      };

      return vk::createInstance(instance_info);
   }

   vk::SurfaceKHR application::create_surface() const
   {
      if (VkSurfaceKHR surface;
         glfwCreateWindowSurface(instance_, window_.get(), nullptr, &surface) == VkResult::VK_SUCCESS)
         return surface;

      throw std::runtime_error("failed to create window surface!");
   }

   vk::PhysicalDevice application::pick_physical_device() const
   {
      std::vector<vk::PhysicalDevice> const physical_devices{ instance_.enumeratePhysicalDevices() };
      if (physical_devices.empty())
         throw std::runtime_error("no physical device with Vulkan support found!");

      for (vk::PhysicalDevice const physical_device : physical_devices)
         if (physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            return physical_device;

      return physical_devices.front();
   }

   std::uint32_t application::graphics_queue_index() const
   {
      std::uint32_t queue_index{};
      for (vk::QueueFamilyProperties const& queue : physical_device_.getQueueFamilyProperties())
         if (queue.queueFlags & vk::QueueFlagBits::eGraphics)
            return queue_index;
         else
            ++queue_index;

      throw std::runtime_error("no queue with support for graphics operations present!");
   }

   std::uint32_t application::presentation_queue_index() const
   {
      for (std::uint32_t queue_index{}; queue_index < physical_device_.getQueueFamilyProperties().size(); ++queue_index)
         if (physical_device_.getSurfaceSupportKHR(queue_index, surface_))
            return queue_index;
         else
            ++queue_index;

      throw std::runtime_error("no queue with support for surface presentation present!");
   }

   vk::Device application::create_device() const
   {
      std::array constexpr queue_priorities{ 1.0f };

      std::vector queue_infos{
         vk::DeviceQueueCreateInfo{
            .queueFamilyIndex{ graphics_queue_index_ },
            .queueCount{ 1 },
            .pQueuePriorities{ queue_priorities.data() }
         },
         vk::DeviceQueueCreateInfo{
            .queueFamilyIndex{ presentation_queue_index_ },
            .queueCount{ 1 },
            .pQueuePriorities{ queue_priorities.data() }
         }
      };

      std::ranges::sort(queue_infos);
      auto const& [new_end, old_end] { std::ranges::unique(queue_infos) };
      queue_infos.erase(new_end, old_end);

      std::array constexpr extension_names{ VK_KHR_SURFACE_EXTENSION_NAME };
      
      vk::PhysicalDeviceFeatures constexpr features{};

      // TODO: for backwards comaptibility, enable the same
      // validation layers on each logical device as
      // the ones enabled on the instance from which the
      // logical device is created
      vk::DeviceCreateInfo const device_info{
         .queueCreateInfoCount{ static_cast<std::uint32_t>(queue_infos.size()) },
         .pQueueCreateInfos{ queue_infos.data() },
         .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
         .ppEnabledExtensionNames{ extension_names.data() },
         .pEnabledFeatures{ &features }
      };

      return physical_device_.createDevice(device_info);
   }
}