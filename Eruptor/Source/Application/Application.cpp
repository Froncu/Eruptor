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
      instance_.destroy();
   }

   void application::run() const
   {
      while (not glfwWindowShouldClose(window_.get()))
         glfwPollEvents();
   }

   vk::Instance application::create_instance() const
   {
      std::uint32_t extension_count;
      char const* const* const extensions{ glfwGetRequiredInstanceExtensions(&extension_count) };
      std::vector<char const*> const extension_names{ extensions, extensions + extension_count };

#if defined NDEBUG
      std::vector<char const*> const valdiation_layer_names{};
#else
      std::vector const valdiation_layer_names{ "VK_LAYER_KHRONOS_validation" };
#endif

      vk::InstanceCreateInfo const instance_info{
         {},
         {},
         valdiation_layer_names,
         extension_names
      };

      return vk::createInstance(instance_info);
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

   std::optional<std::uint32_t> application::graphics_queue_index() const
   {
      std::uint32_t queue_index{};
      for (vk::QueueFamilyProperties const& queue : physical_device_.getQueueFamilyProperties())
         if (queue.queueFlags & vk::QueueFlagBits::eGraphics)
            return queue_index;
         else
            ++queue_index;

      return std::nullopt;
   }

   vk::Device application::create_device() const
   {
      std::array constexpr queue_priorities{ 1.0f };

      vk::DeviceQueueCreateInfo const queue_info{
         {},
         graphics_queue_index_,
         queue_priorities
      };

      vk::PhysicalDeviceFeatures constexpr features{};

      // TODO: for backwards comaptibility, enable the same
      // validation layers on each logical device as
      // the ones enabled on the instance from which the
      // logical device is created
      vk::DeviceCreateInfo const device_info{
         {},
         queue_info,
         {},
         {},
         &features
      };

      return physical_device_.createDevice(device_info);
   }
}