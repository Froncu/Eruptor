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
      vk::DebugUtilsMessageTypeFlagsEXT const,
      vk::DebugUtilsMessengerCallbackDataEXT const* const callback_data,
      void* const)
   {
      switch (severity)
      {
         case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
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

      std::array constexpr layer_names{
         std::to_array<char const* const>({
            "VK_LAYER_KHRONOS_validation"
         })
      };

      return {
         vulkan_context_, {
            .flags{},
            .pApplicationInfo{ &app_info },
            .enabledLayerCount{ static_cast<std::uint32_t>(std::ranges::size(layer_names)) },
            .ppEnabledLayerNames{ std::ranges::data(layer_names) },
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(extension_names) }
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
               vk::StructureChain const properties{ device.getProperties2() };
               vk::StructureChain const features{ device.getFeatures2() };

               return properties.get().properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu
                  and features.get().features.wideLines;
            })
      };

      if (physical_device == std::ranges::end(physical_devices))
         throw Exception{ "no suitable physical device found!" };

      return *physical_device;
   }

   std::uint32_t Application::queue_family_index() const
   {
      // TODO: use vk::StructureChain
      auto&& queue_family_properties{ physical_device_.getQueueFamilyProperties2() | std::ranges::views::enumerate };
      auto const queue_family{
         std::ranges::find_if(
            queue_family_properties,
            [this](auto&& pair)
            {
               auto&& [index, properties]{ pair };
               return static_cast<bool>(properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
                  and physical_device_.getSurfaceSupportKHR(static_cast<std::uint32_t>(index), surface_);
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

      std::array const device_queue_create_info{
         std::to_array<vk::DeviceQueueCreateInfo>({
            {
               .flags{},
               .queueFamilyIndex{ queue_family_index_ },
               .queueCount{ 1 },
               .pQueuePriorities{ &queue_priority }
            }
         })
      };

      std::array constexpr device_extension_names{
         std::to_array<char const* const>({
            vk::KHRSwapchainExtensionName
         })
      };

      // TODO: for backwards compatibility, the validation layers here should be the same as the ones enabled on the instance
      return
      {
         physical_device_,
         {
            .pNext{ device_feature_chain.get() },
            .queueCreateInfoCount{ static_cast<std::uint32_t>(std::ranges::size(device_queue_create_info)) },
            .pQueueCreateInfos{ std::ranges::data(device_queue_create_info) },
            .enabledLayerCount{},
            .ppEnabledLayerNames{},
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(device_extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(device_extension_names) },
         }
      };
   }

   vk::raii::Queue Application::queue() const
   {
      return { device_, queue_family_index_, 0 };
   }

   vk::SurfaceFormatKHR Application::surface_format() const
   {
      // TODO: use `vk::StructureChain` and `getSurfaceFormats2KHR` for more functionality
      std::vector const available_surface_formats{ physical_device_.getSurfaceFormatsKHR(surface_) };

      if (std::ranges::empty(available_surface_formats))
         throw Exception{ "no surface formats are available!" };

      auto surface_format{
         std::ranges::find_if(
            available_surface_formats,
            [](vk::SurfaceFormatKHR const& available_surface_format)
            {
               auto const [format, color_space]{ available_surface_format };
               return format == vk::Format::eB8G8R8A8Srgb
                  and color_space == vk::ColorSpaceKHR::eSrgbNonlinear;
            })
      };

      if (surface_format == std::ranges::end(available_surface_formats))
         surface_format = std::ranges::begin(available_surface_formats);

      return *surface_format;
   }

   vk::raii::SwapchainKHR Application::swap_chain() const
   {
      // Present mode

      // TODO: use `vk::StructureChain` for more functionality
      std::vector const available_surface_present_modes{ physical_device_.getSurfacePresentModesKHR(surface_) };

      if (std::ranges::empty(available_surface_present_modes))
         throw Exception{ "no present modes are available!" };

      auto surface_present_mode{
         std::ranges::find_if(
            available_surface_present_modes,
            [](vk::PresentModeKHR const& available_surface_present_mode)
            {
               return available_surface_present_mode == vk::PresentModeKHR::eMailbox;
            })
      };

      if (surface_present_mode == std::ranges::end(available_surface_present_modes))
         surface_present_mode = std::ranges::begin(available_surface_present_modes);

      // TODO: use `vk::StructureChain` and `getSurfaceCapabilities2KHR` for more functionality
      vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };

      // Extent

      vk::Extent2D surface_extent;
      if (surface_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()
         and surface_capabilities.currentExtent.height == std::numeric_limits<uint32_t>::max())
      {
         int width, height;
         glfwGetFramebufferSize(&window_.native(), &width, &height);

         surface_extent = {
            std::clamp<uint32_t>(width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height)
         };
      }
      else
         surface_extent = surface_capabilities.currentExtent;

      // Image count

      std::uint32_t minimal_image_count{ std::max(3u, surface_capabilities.minImageCount) };
      if (surface_capabilities.maxImageCount)
         minimal_image_count = std::min(minimal_image_count, surface_capabilities.maxImageCount);

      // Swap chain

      return {
         device_, {
            .surface{ *surface_ },
            .minImageCount{ minimal_image_count },
            .imageFormat{ surface_format_.format },
            .imageColorSpace{ surface_format_.colorSpace },
            .imageExtent{ surface_extent },
            .imageArrayLayers{ 1 },
            .imageUsage{ vk::ImageUsageFlagBits::eColorAttachment },
            .imageSharingMode{ vk::SharingMode::eExclusive },
            .queueFamilyIndexCount{},
            .pQueueFamilyIndices{},
            .preTransform{ surface_capabilities.currentTransform },
            .compositeAlpha{ vk::CompositeAlphaFlagBitsKHR::eOpaque },
            .presentMode{ *surface_present_mode },
            .clipped{ true },
            .oldSwapchain{}
         }
      };
   }

   std::vector<vk::raii::ImageView> Application::swap_chain_image_views() const
   {
      vk::ImageViewCreateInfo create_info{
         .viewType{ vk::ImageViewType::e2D },
         .format{ surface_format_.format },
         .components{
            .r{ vk::ComponentSwizzle::eIdentity },
            .g{ vk::ComponentSwizzle::eIdentity },
            .b{ vk::ComponentSwizzle::eIdentity },
            .a{ vk::ComponentSwizzle::eIdentity }
         },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .baseMipLevel{ 0 },
            .levelCount{ 1 },
            .baseArrayLayer{ 0 },
            .layerCount{ 1 }
         }
      };

      std::vector<vk::raii::ImageView> image_views{};
      image_views.reserve(swap_chain_images_.size());
      for (vk::Image const image : swap_chain_images_)
      {
         create_info.image = image;
         image_views.emplace_back(device_, create_info);
      }

      return image_views;
   }
}