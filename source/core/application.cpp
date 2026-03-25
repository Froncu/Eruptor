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

   Application::~Application()
   {
      vk::Result const result{ device_.waitIdle() };
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to wait idle on the device! ({})", to_string(result)));
   }

   bool Application::tick()
   {
      vk::raii::CommandBuffer const& command_buffer{ command_buffers_[frame_index_] };
      vk::raii::Semaphore const& image_available_semaphore{ image_available_semaphores_[frame_index_] };
      vk::raii::Semaphore const& command_buffer_finished_semaphore{ command_buffer_finished_semaphores_[frame_index_] };
      vk::raii::Fence const& presentation_finished_fence{ presentation_finished_fences_[frame_index_] };

      vk::Result result{ device_.waitForFences(*presentation_finished_fence, {}, std::numeric_limits<uint64_t>::max()) };
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to wait for fence(s)! ({})", to_string(result)));

      result = device_.resetFences(*presentation_finished_fence);
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to reset fence(s)! ({})", to_string(result)));

      vk::ResultValue swap_chain_image_index{
         swap_chain_.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), *image_available_semaphore)
      };

      if (swap_chain_image_index.result == vk::Result::eErrorOutOfDateKHR)
      {
         recreate_swap_chain();
         swap_chain_image_index =
            swap_chain_.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), *image_available_semaphore);
      }

      runtime_assert(swap_chain_image_index.has_value(),
         std::format("failed to acquire next image index! ({})", to_string(swap_chain_image_index.result)));

      result = command_buffer.begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to begin command buffer! ({})", to_string(result)));

      vk::ImageMemoryBarrier2 const begin_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .oldLayout{ vk::ImageLayout::eUndefined },
         .newLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .image{ swap_chain_images_[*swap_chain_image_index] },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2({
         .imageMemoryBarrierCount{ 1 },
         .pImageMemoryBarriers{ &begin_barrier }
      });

      vk::RenderingAttachmentInfo const attachment_info{
         .imageView{ swap_chain_image_views_[*swap_chain_image_index] },
         .imageLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .clearValue{ vk::ClearColorValue{ 0.1f, 0.1f, 0.1f, 1.0f } }
      };

      command_buffer.beginRendering({
         .renderArea{
            .extent{ surface_extent_ }
         },
         .layerCount{ 1 },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &attachment_info }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);
      command_buffer.bindVertexBuffers(0, { vertex_buffer_ }, { 0 });
      command_buffer.setViewport(0, {
         {
            .width{ static_cast<float>(surface_extent_.width) },
            .height{ static_cast<float>(surface_extent_.height) },
            .maxDepth{ 1.0f },
         }
      });

      command_buffer.setScissor(0, {
         {
            .extent{ surface_extent_ }
         }
      });

      command_buffer.draw(3, 1, 0, 0);
      command_buffer.endRendering();

      vk::ImageMemoryBarrier2 const end_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .srcAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eBottomOfPipe },
         .dstAccessMask{ vk::AccessFlagBits2::eNone },
         .oldLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .newLayout{ vk::ImageLayout::ePresentSrcKHR },
         .image{ swap_chain_images_[*swap_chain_image_index] },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier2(
         {
            .imageMemoryBarrierCount{ 1 },
            .pImageMemoryBarriers{ &end_barrier }
         });

      result = command_buffer.end();
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to end command buffer! ({})", to_string(result)));

      std::array const wait_semaphore_infos{
         std::to_array<vk::SemaphoreSubmitInfo>({
            {
               .semaphore{ image_available_semaphore },
               .stageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
            }
         })
      };

      std::array const command_buffer_infos{
         std::to_array<vk::CommandBufferSubmitInfo>({
            {
               .commandBuffer{ command_buffer }
            }
         })
      };

      std::array const signal_semaphore_infos{
         std::to_array<vk::SemaphoreSubmitInfo>({
            {
               .semaphore{ command_buffer_finished_semaphore },
               .stageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
            }
         })
      };

      result = queue_.submit2({
         vk::SubmitInfo2{
            .waitSemaphoreInfoCount{ static_cast<std::uint32_t>(std::ranges::size(wait_semaphore_infos)) },
            .pWaitSemaphoreInfos{ std::ranges::data(wait_semaphore_infos) },
            .commandBufferInfoCount{ static_cast<std::uint32_t>(std::ranges::size(command_buffer_infos)) },
            .pCommandBufferInfos{ std::ranges::data(command_buffer_infos) },
            .signalSemaphoreInfoCount{ static_cast<std::uint32_t>(std::ranges::size(signal_semaphore_infos)) },
            .pSignalSemaphoreInfos{ std::ranges::data(signal_semaphore_infos) }
         }
      });
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to submit command buffer! ({})", to_string(result)));

      vk::SwapchainPresentFenceInfoEXT const swap_chain_present_fence_info{
         .swapchainCount{ 1 },
         .pFences{ &*presentation_finished_fence }
      };

      result = queue_.presentKHR({
         .pNext{ &swap_chain_present_fence_info },
         .waitSemaphoreCount{ 1 },
         .pWaitSemaphores{ &*command_buffer_finished_semaphore },
         .swapchainCount{ 1 },
         .pSwapchains{ &*swap_chain_ },
         .pImageIndices{ &*swap_chain_image_index },
         .pResults{}
      });

      switch (result)
      {
         case vk::Result::eErrorOutOfDateKHR:
            recreate_swap_chain();
            return true;

         default:
            runtime_assert(result == vk::Result::eSuccess,
               std::format("failed to submit command buffer! ({})", to_string(result)));
            break;
      }

      ++frame_index_ %= FRAMES_IN_FLIGHT;
      return true;
   }

   void Application::poll()
   {
      glfwPollEvents();
   }

   Application::Application(std::string_view const name, std::uint32_t const version)
      : instance_{ instance(name, version) }
   {
      vk::Result result{ vertex_buffer_.bindMemory(vertex_buffer_memory_, 0) };
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to bind vertex buffer's memory! ({})", to_string(result)));

      vk::DeviceSize const buffer_size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() };
      vk::raii::Buffer const staging_buffer{
         buffer({
            .size{ buffer_size },
            .usage{ vk::BufferUsageFlagBits::eTransferSrc },
            .sharingMode{ vk::SharingMode::eExclusive }
         })
      };

      vk::raii::DeviceMemory const staging_buffer_memory{
         memory(staging_buffer.getMemoryRequirements(),
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      result = staging_buffer.bindMemory(staging_buffer_memory, 0);
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to bind staging buffer's memory! ({})", to_string(result)));

      vk::ResultValue const mapped_memory{ staging_buffer_memory.mapMemory(0, buffer_size) };
      runtime_assert(mapped_memory.has_value(),
         std::format("failed to map staging buffer's memory! ({})", to_string(mapped_memory.result)));

      std::memcpy(*mapped_memory, vertices_.data(), buffer_size);
      staging_buffer_memory.unmapMemory();

      vk::ResultValue const command_buffers{
         device_.allocateCommandBuffers({
            .commandPool{ command_pool_ },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ 1 }
         })
      };
      runtime_assert(command_buffers.has_value(),
         std::format("failed to allocate a command buffer! ({})", to_string(command_buffers.result)));

      result = command_buffers->front().begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to begin command buffer! ({})", to_string(result)));

      command_buffers->front().copyBuffer(
         staging_buffer,
         vertex_buffer_,
         {
            {
               .size{ buffer_size }
            }
         });

      result = command_buffers->front().end();
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to end command buffer! ({})", to_string(result)));

      queue_.submit({
         {
            {
               .commandBufferCount{ 1 },
               .pCommandBuffers{ &*command_buffers->front() },
            }
         }
      });
      result = queue_.waitIdle();
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to wait for queue! ({})", to_string(result)));
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

      std::vector<char const*> layer_names{};
      if constexpr (DEBUG_BUILD)
         layer_names.push_back("VK_LAYER_KHRONOS_validation");

      vk::ResultValue instance{
         vulkan_context_.createInstance({
            .flags{},
            .pApplicationInfo{ &app_info },
            .enabledLayerCount{ static_cast<std::uint32_t>(std::ranges::size(layer_names)) },
            .ppEnabledLayerNames{ std::ranges::data(layer_names) },
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(extension_names) }
         })
      };
      runtime_assert(instance.has_value(),
         std::format("failed to create a Vulkan instance! ({})", to_string(instance.result)));

      return std::move(*instance);
   }

   vk::raii::DebugUtilsMessengerEXT Application::debug_messenger() const
   {
      vk::ResultValue debug_messenger{
         instance_.createDebugUtilsMessengerEXT({
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
         })
      };
      runtime_assert(debug_messenger.has_value(),
         std::format("failed to create a debug messenger! ({})", to_string(debug_messenger.result)));

      return std::move(*debug_messenger);
   }

   vk::raii::SurfaceKHR Application::surface() const
   {
      VkSurfaceKHR surface;
      glfwCreateWindowSurface(*instance_, &window_.native(), nullptr, &surface);
      return { instance_, surface };
   }

   vk::raii::PhysicalDevice Application::physical_device() const
   {
      vk::ResultValue const physical_devices{ instance_.enumeratePhysicalDevices() };
      runtime_assert(physical_devices.has_value(),
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

      runtime_assert(physical_device not_eq std::ranges::end(*physical_devices),
         "no suitable physical device found!");

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

               vk::ResultValue const surface_support{
                  physical_device_.getSurfaceSupportKHR(static_cast<std::uint32_t>(index), surface_)
               };
               runtime_assert(surface_support.has_value(),
                  std::format("failed to query for surface support! ({})", to_string(surface_support.result)));

               return *surface_support
                  and static_cast<bool>(properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
            })
      };

      runtime_assert(queue_family not_eq std::ranges::end(queue_family_properties),
         "no suitable queue family found!");

      return static_cast<std::uint32_t>(queue_family.index());
   }

   vk::raii::Device Application::device() const
   {
      vk::StructureChain<
         vk::PhysicalDeviceFeatures2,
         vk::PhysicalDeviceVulkan11Features,
         vk::PhysicalDeviceVulkan13Features,
         vk::PhysicalDeviceVulkan14Features,
         vk::PhysicalDeviceSwapchainMaintenance1FeaturesEXT> const device_feature_chain{
         {
         },
         {
            .shaderDrawParameters{ true },
         },
         {
            .synchronization2{ true },
            .dynamicRendering{ true }
         },
         {
            .maintenance5{ true }
         },
         {
            .swapchainMaintenance1{ true }
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
      vk::ResultValue device{
         physical_device_.createDevice({
            .pNext{ device_feature_chain.get() },
            .queueCreateInfoCount{ static_cast<std::uint32_t>(std::ranges::size(device_queue_create_info)) },
            .pQueueCreateInfos{ std::ranges::data(device_queue_create_info) },
            .enabledLayerCount{},
            .ppEnabledLayerNames{},
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(device_extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(device_extension_names) },
         })
      };
      runtime_assert(device.has_value(),
         std::format("failed to create a device! ({})", to_string(device.result)));

      return std::move(*device);
   }

   vk::raii::Queue Application::queue() const
   {
      return device_.getQueue2({
         .queueFamilyIndex{ queue_family_index_ },
         .queueIndex{ 0 }
      });
   }

   vk::SurfaceFormatKHR Application::surface_format() const
   {
      // TODO: use `vk::StructureChain` and `getSurfaceFormats2KHR` for more functionality
      vk::ResultValue const available_surface_formats{ physical_device_.getSurfaceFormatsKHR(surface_) };
      runtime_assert(available_surface_formats.has_value(),
         std::format("failed to query available surface formats! ({})", to_string(available_surface_formats.result)));

      if (std::ranges::empty(*available_surface_formats))
         throw Exception{ "no surface formats are available!" };

      auto surface_format{
         std::ranges::find_if(
            *available_surface_formats,
            [](vk::SurfaceFormatKHR const& available_surface_format)
            {
               auto const [format, color_space]{ available_surface_format };
               return format == vk::Format::eB8G8R8A8Srgb
                  and color_space == vk::ColorSpaceKHR::eSrgbNonlinear;
            })
      };

      if (surface_format == std::ranges::end(*available_surface_formats))
         surface_format = std::ranges::begin(*available_surface_formats);

      return *surface_format;
   }

   vk::Extent2D Application::surface_extent() const
   {
      vk::ResultValue const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };
      runtime_assert(surface_capabilities.has_value(),
         std::format("failed to query surface capabilities! ({})", to_string(surface_capabilities.result)));

      if (surface_capabilities->currentExtent.width == std::numeric_limits<std::uint32_t>::max()
         and surface_capabilities->currentExtent.height == std::numeric_limits<std::uint32_t>::max())
      {
         int width, height;
         glfwGetFramebufferSize(&window_.native(), &width, &height);

         return {
            std::clamp<std::uint32_t>(width, surface_capabilities->minImageExtent.width,
               surface_capabilities->maxImageExtent.width),
            std::clamp<std::uint32_t>(height, surface_capabilities->minImageExtent.height,
               surface_capabilities->maxImageExtent.height)
         };
      }

      return surface_capabilities->currentExtent;
   }

   vk::raii::SwapchainKHR Application::swap_chain() const
   {
      // TODO: use `vk::StructureChain` for more functionality
      vk::ResultValue const available_surface_present_modes{ physical_device_.getSurfacePresentModesKHR(surface_) };
      runtime_assert(available_surface_present_modes.has_value(),
         std::format("failed to query available surface present modes! ({})",
            to_string(available_surface_present_modes.result)));

      runtime_assert(not std::ranges::empty(*available_surface_present_modes),
         "no present modes are available!");

      auto surface_present_mode{
         std::ranges::find_if(
            *available_surface_present_modes,
            [](vk::PresentModeKHR const& available_surface_present_mode)
            {
               return available_surface_present_mode == vk::PresentModeKHR::eMailbox;
            })
      };

      if (surface_present_mode == std::ranges::end(*available_surface_present_modes))
         surface_present_mode = std::ranges::begin(*available_surface_present_modes);

      // TODO: use `vk::StructureChain` and `getSurfaceCapabilities2KHR` for more functionality
      vk::ResultValue const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };
      runtime_assert(surface_capabilities.has_value(),
         std::format("failed to query surface capabilities! ({})", to_string(surface_capabilities.result)));

      std::uint32_t minimal_image_count{ std::max(3u, surface_capabilities->minImageCount) };
      if (surface_capabilities->maxImageCount)
         minimal_image_count = std::min(minimal_image_count, surface_capabilities->maxImageCount);

      // TODO: make use of `oldSwapchain`
      vk::ResultValue swap_chain{
         device_.createSwapchainKHR({
            .surface{ *surface_ },
            .minImageCount{ minimal_image_count },
            .imageFormat{ surface_format_.format },
            .imageColorSpace{ surface_format_.colorSpace },
            .imageExtent{ surface_extent_ },
            .imageArrayLayers{ 1 },
            .imageUsage{ vk::ImageUsageFlagBits::eColorAttachment },
            .imageSharingMode{ vk::SharingMode::eExclusive },
            .queueFamilyIndexCount{},
            .pQueueFamilyIndices{},
            .preTransform{ surface_capabilities->currentTransform },
            .compositeAlpha{ vk::CompositeAlphaFlagBitsKHR::eOpaque },
            .presentMode{ *surface_present_mode },
            .clipped{ true },
            .oldSwapchain{}
         })
      };
      runtime_assert(swap_chain.has_value(),
         std::format("failed to create a swap chain! ({})", to_string(swap_chain.result)));

      return std::move(*swap_chain);
   }

   std::vector<vk::Image> Application::swap_chain_images() const
   {
      vk::ResultValue swap_chain_images{ swap_chain_.getImages() };
      runtime_assert(swap_chain_images.has_value(),
         std::format("failed to retrieve swap chain images! ({})", to_string(swap_chain_images.result)));

      return std::move(*swap_chain_images);
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
         vk::ResultValue image_view{ device_.createImageView(create_info) };
         runtime_assert(image_view.has_value(),
            std::format("failed to create a swap chain image view! ({})", to_string(image_view.result)));

         image_views.push_back(std::move(*image_view));
      }

      return image_views;
   }

   vk::raii::PipelineLayout Application::pipeline_layout() const
   {
      vk::ResultValue pipeline_layout{ device_.createPipelineLayout({}) };
      runtime_assert(pipeline_layout.has_value(),
         std::format("failed to create a pipeline layout! ({})", to_string(pipeline_layout.result)));

      return std::move(pipeline_layout.value);
   }

   vk::raii::Pipeline Application::pipeline() const
   {
      Slang::ComPtr<slang::IGlobalSession> global_session;
      createGlobalSession(global_session.writeRef());
      runtime_assert(global_session,
         std::format("failed to create a global session! ({})", slang::getLastInternalErrorMessage()));

      std::array slang_targets{
         std::to_array<slang::TargetDesc>({
            {
               .format{ SLANG_SPIRV }
            }
         })
      };

      slang::SessionDesc const slang_session_description{
         .targets{ std::ranges::data(slang_targets) },
         .targetCount{ static_cast<SlangInt>(std::ranges::size(slang_targets)) },
         .defaultMatrixLayoutMode{ SLANG_MATRIX_LAYOUT_COLUMN_MAJOR }
      };

      Slang::ComPtr<slang::ISession> slang_session;
      global_session->createSession(slang_session_description, slang_session.writeRef());
      runtime_assert(slang_session,
         std::format("failed to create a slang session! ({})", slang::getLastInternalErrorMessage()));

      Slang::ComPtr const slang_module{
         slang_session->loadModuleFromSource("shader", "assets/shaders/shader.slang", nullptr, nullptr)
      };
      runtime_assert(slang_module,
         std::format("failed to load a slang module! ({})", slang::getLastInternalErrorMessage()));

      Slang::ComPtr<ISlangBlob> spirv;
      slang_module->getTargetCode(0, spirv.writeRef());
      runtime_assert(spirv,
         std::format("failed to load get target code! ({})", slang::getLastInternalErrorMessage()));

      vk::ShaderModuleCreateInfo const shader_module_create_info{
         .codeSize{ spirv->getBufferSize() },
         .pCode{ static_cast<std::uint32_t const* const>(spirv->getBufferPointer()) }
      };

      std::array const shader_stage_create_infos{
         std::to_array<vk::PipelineShaderStageCreateInfo>({
            {
               .pNext{ &shader_module_create_info },
               .stage{ vk::ShaderStageFlagBits::eVertex },
               .pName{ "vertMain" }
            },
            {
               .pNext{ &shader_module_create_info },
               .stage{ vk::ShaderStageFlagBits::eFragment },
               .pName{ "fragMain" }
            }
         })
      };

      std::array constexpr dynamic_states{
         vk::DynamicState::eViewport,
         vk::DynamicState::eScissor
      };

      vk::PipelineDynamicStateCreateInfo const dynamic_state_create_info{
         .dynamicStateCount{ static_cast<uint32_t>(std::ranges::size(dynamic_states)) },
         .pDynamicStates{ std::ranges::data(dynamic_states) }
      };

      vk::PipelineVertexInputStateCreateInfo constexpr vertex_input_state_create_info{
         .vertexBindingDescriptionCount{ static_cast<std::uint32_t>(std::ranges::size(Vertex::INPUT_BINDING_DESCRIPTIONS)) },
         .pVertexBindingDescriptions{ std::ranges::data(Vertex::INPUT_BINDING_DESCRIPTIONS) },
         .vertexAttributeDescriptionCount{
            static_cast<std::uint32_t>(std::ranges::size(Vertex::INPUT_ATTRIBUTE_DESCRIPTIONS))
         },
         .pVertexAttributeDescriptions{ std::ranges::data(Vertex::INPUT_ATTRIBUTE_DESCRIPTIONS) }
      };

      vk::PipelineInputAssemblyStateCreateInfo constexpr input_assembly_state_create_info{
         .topology{ vk::PrimitiveTopology::eTriangleList }
      };

      vk::PipelineViewportStateCreateInfo constexpr viewport_state_create_info{
         .viewportCount{ 1 },
         .scissorCount{ 1 }
      };

      vk::PipelineRasterizationStateCreateInfo constexpr rasterization_state_create_info{
         .depthClampEnable{ vk::False },
         .rasterizerDiscardEnable{ vk::False },
         .polygonMode{ vk::PolygonMode::eFill },
         .cullMode{ vk::CullModeFlagBits::eBack },
         .frontFace{ vk::FrontFace::eClockwise },
         .depthBiasEnable{ vk::False },
         .depthBiasSlopeFactor{ 1.0f },
         .lineWidth{ 1.0f }
      };

      vk::PipelineMultisampleStateCreateInfo constexpr multisample_state_create_info{
         .rasterizationSamples{ vk::SampleCountFlagBits::e1 },
         .sampleShadingEnable{ vk::False }
      };

      std::array constexpr color_blend_attachment_state{
         std::to_array<vk::PipelineColorBlendAttachmentState>({
            {
               .blendEnable{ vk::False },
               .colorWriteMask{
                  vk::ColorComponentFlagBits::eR |
                  vk::ColorComponentFlagBits::eG |
                  vk::ColorComponentFlagBits::eB |
                  vk::ColorComponentFlagBits::eA
               }
            }
         })
      };

      vk::PipelineColorBlendStateCreateInfo const color_blend_state_create_info{
         .logicOpEnable{ vk::False },
         .logicOp{ vk::LogicOp::eCopy },
         .attachmentCount{ static_cast<std::uint32_t>(std::ranges::size(color_blend_attachment_state)) },
         .pAttachments{ std::ranges::data(color_blend_attachment_state) }
      };

      std::array const color_attachments{
         surface_format_.format,
      };

      vk::PipelineRenderingCreateInfo const pipeline_rendering_create_info{
         .colorAttachmentCount{ static_cast<std::uint32_t>(std::ranges::size(color_attachments)) },
         .pColorAttachmentFormats{ std::ranges::data(color_attachments) }
      };

      vk::ResultValue pipeline{
         device_.createGraphicsPipeline(nullptr, {
            .pNext{ &pipeline_rendering_create_info },
            .stageCount{ static_cast<std::uint32_t>(std::ranges::size(shader_stage_create_infos)) },
            .pStages{ std::ranges::data(shader_stage_create_infos) },
            .pVertexInputState{ &vertex_input_state_create_info },
            .pInputAssemblyState{ &input_assembly_state_create_info },
            .pViewportState{ &viewport_state_create_info },
            .pRasterizationState{ &rasterization_state_create_info },
            .pMultisampleState{ &multisample_state_create_info },
            .pColorBlendState{ &color_blend_state_create_info },
            .pDynamicState{ &dynamic_state_create_info },
            .layout{ pipeline_layout_ },
         })
      };
      runtime_assert(pipeline.has_value(),
         std::format("failed create a pipeline! ({})", to_string(pipeline.result)));

      return std::move(*pipeline);
   }

   vk::raii::CommandPool Application::command_pool() const
   {
      vk::ResultValue command_pool{
         device_.createCommandPool({
            .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
            .queueFamilyIndex{ queue_family_index_ }
         })
      };
      runtime_assert(command_pool.has_value(),
         std::format("failed create a command pool! ({})", to_string(command_pool.result)));

      return std::move(*command_pool);
   }

   vk::raii::CommandBuffers Application::command_buffers() const
   {
      vk::ResultValue command_buffers{
         device_.allocateCommandBuffers({
            .commandPool{ command_pool_ },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ FRAMES_IN_FLIGHT }
         })
      };
      runtime_assert(command_buffers.has_value(),
         std::format("failed allocate command buffers! ({})", to_string(command_buffers.result)));

      return std::move(*command_buffers);
   }

   std::vector<vk::raii::Semaphore> Application::semaphores() const
   {
      std::vector<vk::raii::Semaphore> semaphores{};
      semaphores.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
      {
         vk::ResultValue semaphore{ device_.createSemaphore({}) };
         runtime_assert(semaphore.has_value(),
            std::format("failed create a semaphore! ({})", to_string(semaphore.result)));

         semaphores.push_back(std::move(*semaphore));
      }

      return semaphores;
   }

   std::vector<vk::raii::Fence> Application::fences() const
   {
      std::vector<vk::raii::Fence> fences{};
      fences.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
      {
         vk::ResultValue fence{
            device_.createFence({
               .flags{ vk::FenceCreateFlagBits::eSignaled }
            })
         };
         runtime_assert(fence.has_value(),
            std::format("failed create a fence! ({})", to_string(fence.result)));

         fences.push_back(std::move(*fence));
      }

      return fences;
   }

   vk::raii::Buffer Application::buffer(vk::BufferCreateInfo const& create_info) const
   {
      vk::ResultValue buffer{ device_.createBuffer(create_info) };
      runtime_assert(buffer.has_value(),
         std::format("failed to create vertex buffer! ({})", to_string(buffer.result)));

      return std::move(*buffer);
   }

   vk::raii::DeviceMemory Application::memory(vk::MemoryRequirements const& requirements,
      vk::MemoryPropertyFlags const properties) const
   {
      vk::PhysicalDeviceMemoryProperties const available_properties{ physical_device_.getMemoryProperties() };
      std::bitset<sizeof(requirements.memoryTypeBits) * 8> const type_bits{ requirements.memoryTypeBits };

      std::uint32_t index{};
      for (; index < available_properties.memoryTypeCount; ++index)
         if (type_bits[index]
            and (available_properties.memoryTypes[index].propertyFlags & properties) == properties)
            break;

      vk::ResultValue device_memory{
         device_.allocateMemory({
            .allocationSize{ requirements.size },
            .memoryTypeIndex{ index }
         })
      };
      runtime_assert(device_memory.has_value(),
         std::format("failed to allocate memory! ({})", to_string(device_memory.result)));

      return std::move(*device_memory);
   }

   vk::raii::Buffer Application::vertex_buffer() const
   {
      return buffer({
         .size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() },
         .usage{ vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst },
         .sharingMode{ vk::SharingMode::eExclusive }
      });
   }

   vk::raii::DeviceMemory Application::vertex_buffer_memory() const
   {
      return memory(vertex_buffer_.getMemoryRequirements(), vk::MemoryPropertyFlagBits::eDeviceLocal);
   }

   void Application::recreate_swap_chain()
   {
      surface_extent_ = surface_extent();
      swap_chain_.clear();
      swap_chain_ = swap_chain();
      swap_chain_images_ = swap_chain_images();
      swap_chain_image_views_ = swap_chain_image_views();
   }
}