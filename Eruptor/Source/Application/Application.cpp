#include "erupch.hpp"

#include "application.hpp"
#include "shader_compiler/shader_compiler.hpp"

namespace eru
{
   application::application()
   {
   }

   application::~application()
   {
      for (vk::ImageView const image_view : swap_chain_image_views_)
         device_.destroyImageView(image_view);

      device_.destroySwapchainKHR(swap_chain_);
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

      return vk::createInstance({
         .enabledLayerCount{ static_cast<std::uint32_t>(valdiation_layer_names.size()) },
         .ppEnabledLayerNames{ valdiation_layer_names.data() },
         .enabledExtensionCount{ extension_count },
         .ppEnabledExtensionNames{ extension_names }
         });
   }

   vk::SurfaceKHR application::create_surface() const
   {
      if (VkSurfaceKHR surface;
         glfwCreateWindowSurface(instance_, window_.get(), nullptr, &surface) == VkResult::VK_SUCCESS)
         return static_cast<vk::SurfaceKHR>(surface);

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

      std::array constexpr extension_names{ vk::KHRSwapchainExtensionName };

      // TODO: for backwards comaptibility, enable the same
      // validation layers on each logical device as
      // the ones enabled on the instance from which the
      // logical device is created
      return physical_device_.createDevice({
         .queueCreateInfoCount{ static_cast<std::uint32_t>(queue_infos.size()) },
         .pQueueCreateInfos{ queue_infos.data() },
         .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
         .ppEnabledExtensionNames{ extension_names.data() }
         });
   }

   vk::SurfaceFormatKHR application::pick_swap_chain_format() const
   {
      auto const available_formats{ physical_device_.getSurfaceFormatsKHR(surface_) };
      for (vk::SurfaceFormatKHR const available_format : available_formats)
         if (available_format.format == vk::Format::eB8G8R8A8Srgb and
            available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return available_format;

      return available_formats.front();
   }

   vk::Extent2D application::pick_swap_chain_extent() const
   {
      vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };

      if (surface_capabilities.currentExtent.width not_eq std::numeric_limits<std::uint32_t>::max())
         return surface_capabilities.currentExtent;
      else
      {
         int width;
         int height;
         glfwGetFramebufferSize(window_.get(), &width, &height);

         return{
            .width{
               std::clamp(static_cast<std::uint32_t>(width),
               surface_capabilities.minImageExtent.width,
               surface_capabilities.maxImageExtent.width)
            },
            .height{
               std::clamp(static_cast<std::uint32_t>(height),
               surface_capabilities.minImageExtent.height,
               surface_capabilities.maxImageExtent.height)
            }
         };
      }
   }

   vk::SwapchainKHR application::create_swap_chain() const
   {
      // NOTE: eFifo is guaranteed to be present, 
      // so we use it as a standard value
      vk::PresentModeKHR present_mode{ vk::PresentModeKHR::eFifo };
      for (auto const avaialble_present_mode : physical_device_.getSurfacePresentModesKHR(surface_))
         if (avaialble_present_mode == vk::PresentModeKHR::eMailbox)
         {
            present_mode = avaialble_present_mode;
            break;
         }

      vk::SharingMode sharing_mode{ vk::SharingMode::eExclusive };
      std::vector<std::uint32_t> queue_family_indices{};

      if (graphics_queue_index_ not_eq presentation_queue_index_)
      {
         sharing_mode = vk::SharingMode::eConcurrent;

         queue_family_indices.resize(2);
         queue_family_indices[0] = graphics_queue_index_;
         queue_family_indices[1] = presentation_queue_index_;
      }

      vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };
      return device_.createSwapchainKHR({
         .surface{ surface_ },
         .minImageCount{
            surface_capabilities.maxImageCount not_eq 0 ?
            std::min(surface_capabilities.minImageCount + 1, surface_capabilities.maxImageCount) :
            surface_capabilities.minImageCount + 1
         },
         .imageFormat{ swap_chain_format_.format },
         .imageColorSpace{ swap_chain_format_.colorSpace },
         .imageExtent{ swap_chain_extent_ },
         .imageArrayLayers{ 1 },
         .imageUsage{ vk::ImageUsageFlagBits::eColorAttachment },
         .imageSharingMode{ sharing_mode },
         .queueFamilyIndexCount{ static_cast<std::uint32_t>(queue_family_indices.size()) },
         .pQueueFamilyIndices{ queue_family_indices.data() },
         .preTransform{ surface_capabilities.currentTransform },
         .compositeAlpha{ vk::CompositeAlphaFlagBitsKHR::eOpaque },
         .presentMode{ present_mode },
         .clipped{ true }
         });
   }

   std::vector<vk::ImageView> application::create_image_views() const
   {
      std::vector<vk::ImageView> image_views{};
      image_views.reserve(swap_chain_images_.size());

      for (vk::Image const image : swap_chain_images_)
         image_views.emplace_back(device_.createImageView({
               .image{ image },
               .viewType{ vk::ImageViewType::e2D },
               .format{ swap_chain_format_.format },
               .components{
                  .r{ vk::ComponentSwizzle::eIdentity },
                  .g{ vk::ComponentSwizzle::eIdentity },
                  .b{ vk::ComponentSwizzle::eIdentity },
                  .a{ vk::ComponentSwizzle::eIdentity }
               },
               .subresourceRange{
                  .aspectMask{ vk::ImageAspectFlagBits::eColor },
                  .levelCount{ 1 },
                  .layerCount{ 1 }
               }
            }));

      return image_views;
   }

   vk::ShaderModule application::create_shader_module(std::vector<std::uint32_t> const& byte_code) const
   {
      return device_.createShaderModule({
            .codeSize{ byte_code.size() },
            .pCode{ byte_code.data() }
         });
   }

   vk::Pipeline application::create_pipeline() const
   {
      vk::ShaderModule const vertex_shader_module{ create_shader_module(compile_shader("resources/shaders/shader.vert")) };
      vk::ShaderModule const fragment_shader_module{ create_shader_module(compile_shader("resources/shaders/shader.frag")) };

      std::array const shader_stage_create_infos{
         vk::PipelineShaderStageCreateInfo{
            .stage{ vk::ShaderStageFlagBits::eVertex },
            .module{ vertex_shader_module },
            .pName{ "main" }
         },
         vk::PipelineShaderStageCreateInfo{
            .stage{ vk::ShaderStageFlagBits::eFragment },
            .module{ fragment_shader_module },
            .pName{ "main" }
         }
      };

      device_.destroyShaderModule(fragment_shader_module);
      device_.destroyShaderModule(vertex_shader_module);
   }
}