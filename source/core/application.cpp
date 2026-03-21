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
      device_.waitIdle();
   }

   bool Application::tick()
   {
      vk::raii::CommandBuffer const& command_buffer{ command_buffers_[frame_index_] };
      vk::raii::Semaphore const& image_available_semaphore{ image_available_semaphores_[frame_index_] };
      vk::raii::Semaphore const& command_buffer_finished_semaphore{ command_buffer_finished_semaphores_[frame_index_] };
      vk::raii::Fence const& presentation_finished_fence{ presentation_finished_fences_[frame_index_] };

      if (device_.waitForFences(*presentation_finished_fence, {}, std::numeric_limits<uint64_t>::max()) not_eq
         vk::Result::eSuccess)
         throw Exception{ "failed to wait for fence!" };
      device_.resetFences(*presentation_finished_fence);

      auto [swap_chain_result, swap_chain_image_index]{
         swap_chain_.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), *image_available_semaphore)
      };

      command_buffer.begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });

      vk::ImageMemoryBarrier2 const begin_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits2::eColorAttachmentWrite },
         .oldLayout{ vk::ImageLayout::eUndefined },
         .newLayout{ vk::ImageLayout::eColorAttachmentOptimal },
         .image{ swap_chain_images_[swap_chain_image_index] },
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
         .imageView{ swap_chain_image_views_[swap_chain_image_index] },
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
         .image{ swap_chain_images_[swap_chain_image_index] },
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

      command_buffer.end();

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

      queue_.submit2({
         vk::SubmitInfo2{
            .waitSemaphoreInfoCount{ static_cast<std::uint32_t>(std::ranges::size(wait_semaphore_infos)) },
            .pWaitSemaphoreInfos{ std::ranges::data(wait_semaphore_infos) },
            .commandBufferInfoCount{ static_cast<std::uint32_t>(std::ranges::size(command_buffer_infos)) },
            .pCommandBufferInfos{ std::ranges::data(command_buffer_infos) },
            .signalSemaphoreInfoCount{ static_cast<std::uint32_t>(std::ranges::size(signal_semaphore_infos)) },
            .pSignalSemaphoreInfos{ std::ranges::data(signal_semaphore_infos) }
         }
      });

      vk::SwapchainPresentFenceInfoEXT const swap_chain_present_fence_info{
         .swapchainCount{ 1 },
         .pFences{ &*presentation_finished_fence }
      };

      vk::Result const present_result{
         queue_.presentKHR({
            .pNext{ &swap_chain_present_fence_info },
            .waitSemaphoreCount{ 1 },
            .pWaitSemaphores{ &*command_buffer_finished_semaphore },
            .swapchainCount{ 1 },
            .pSwapchains{ &*swap_chain_ },
            .pImageIndices{ &swap_chain_image_index },
            .pResults{}
         })
      };

      switch (present_result)
      {
         case vk::Result::eSuccess:
            break;

         case vk::Result::eSuboptimalKHR:
            Locator::get<Logger>().warning("vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR!");
            break;

         default:
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

   vk::Extent2D Application::surface_extent() const
   {
      vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };
      if (surface_capabilities.currentExtent.width == std::numeric_limits<std::uint32_t>::max()
         and surface_capabilities.currentExtent.height == std::numeric_limits<std::uint32_t>::max())
      {
         int width, height;
         glfwGetFramebufferSize(&window_.native(), &width, &height);

         return {
            std::clamp<std::uint32_t>(width, surface_capabilities.minImageExtent.width,
               surface_capabilities.maxImageExtent.width),
            std::clamp<std::uint32_t>(height, surface_capabilities.minImageExtent.height,
               surface_capabilities.maxImageExtent.height)
         };
      }

      return surface_capabilities.currentExtent;
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

      // Image count

      // TODO: use `vk::StructureChain` and `getSurfaceCapabilities2KHR` for more functionality
      vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };
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
            .imageExtent{ surface_extent_ },
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

   vk::raii::PipelineLayout Application::pipeline_layout() const
   {
      return {
         device_,
         vk::PipelineLayoutCreateInfo{}
      };
   }

   vk::raii::Pipeline Application::pipeline() const
   {
      Slang::ComPtr<slang::IGlobalSession> global_session;
      createGlobalSession(global_session.writeRef());

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

      Slang::ComPtr const slang_module{
         slang_session->loadModuleFromSource("shader", "assets/shaders/shader.slang", nullptr, nullptr)
      };

      Slang::ComPtr<ISlangBlob> spirv;
      slang_module->getTargetCode(0, spirv.writeRef());

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

      vk::PipelineVertexInputStateCreateInfo constexpr vertex_input_state_create_info{};

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

      return {
         device_,
         nullptr,
         {
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
         }
      };
   }

   vk::raii::CommandPool Application::command_pool() const
   {
      return {
         device_,
         vk::CommandPoolCreateInfo{
            .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
            .queueFamilyIndex{ queue_family_index_ }
         }
      };
   }

   vk::raii::CommandBuffers Application::command_buffers() const
   {
      return {
         device_,
         {
            .commandPool{ command_pool_ },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ FRAMES_IN_FLIGHT }
         }
      };
   }

   std::vector<vk::raii::Semaphore> Application::semaphores() const
   {
      std::vector<vk::raii::Semaphore> semaphores{};
      semaphores.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
         semaphores.emplace_back(device_, vk::SemaphoreCreateInfo{});

      return semaphores;
   }

   std::vector<vk::raii::Fence> Application::fences() const
   {
      std::vector<vk::raii::Fence> fences{};
      fences.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
         fences.emplace_back(device_, vk::FenceCreateInfo{
            .flags{ vk::FenceCreateFlagBits::eSignaled }
         });

      return fences;
   }
}