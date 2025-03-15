#include "application.hpp"
#include "shader_compiler/shader_compiler.hpp"

namespace eru
{
   application::~application()
   {
      for (vk::Fence const fence : command_buffer_executed_fences_)
         device_.destroyFence(fence);

      for (vk::Semaphore const semaphore : render_finished_semaphores_)
         device_.destroySemaphore(semaphore);

      for (vk::Semaphore const semaphore : image_available_semaphores_)
         device_.destroySemaphore(semaphore);

      device_.destroyCommandPool(command_pool_);

      device_.destroyPipeline(pipeline_);
      device_.destroyPipelineLayout(pipeline_layout_);
      for (vk::Framebuffer const framebuffer : swap_chain_framebuffers_)
         device_.destroyFramebuffer(framebuffer);
      device_.destroyRenderPass(render_pass_);

      for (vk::ImageView const image_view : swap_chain_image_views_)
         device_.destroyImageView(image_view);
      device_.destroySwapchainKHR(swap_chain_);

      device_.destroy();

      instance_.destroySurfaceKHR(surface_);

      if constexpr (USE_VALIDATION_LAYERS)
         instance_.destroyDebugUtilsMessengerEXT(debug_messenger_, nullptr, dispatch_loader_dynamic_);

      instance_.destroy();
   }

   void application::run()
   {
      auto loop{ true };

      SDL_InitSubSystem(SDL_INIT_EVENTS);
      while (loop)
      {
         SDL_Event event;
         while (SDL_PollEvent(&event))
            switch (event.type)
            {
               case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                  loop = false;
                  break;

               case SDL_EVENT_WINDOW_MINIMIZED:
               case SDL_EVENT_WINDOW_RESIZED:
                  recreate_swapchain();
                  break;

               default:
                  break;
            }

         draw_frame();
      }
      SDL_QuitSubSystem(SDL_INIT_EVENTS);

      device_.waitIdle();
   }

   vk::Instance application::create_instance()
   {
      auto constexpr validation_layer_names{
         []
         {
            if constexpr (USE_VALIDATION_LAYERS)
               return std::array{ "VK_LAYER_KHRONOS_validation" };
            else
               return std::array<char const*, 0>{};
         }()
      };

      std::uint32_t sdl_extension_count;
      char const* const* const sdl_extension_names{ SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count) };
      std::vector<char const*> extension_names{ sdl_extension_names, sdl_extension_names + sdl_extension_count };

      if constexpr (USE_VALIDATION_LAYERS)
         extension_names.push_back(vk::EXTDebugUtilsExtensionName);

      try
      {
         return vk::createInstance({
            .enabledLayerCount{ static_cast<std::uint32_t>(validation_layer_names.size()) },
            .ppEnabledLayerNames{ validation_layer_names.data() },
            .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
            .ppEnabledExtensionNames{ extension_names.data() }
         });
      }
      catch (vk::LayerNotPresentError const&)
      {
         // TODO: the debug messenger as well as the needed for it extension are still enabled
         std::cout << "failed to find one or more validation layers - continuing without the following:\n";
         for (std::string_view const validation_layer_name : validation_layer_names)
            std::cout << std::format("- {}\n", validation_layer_name);

         return vk::createInstance({
            .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
            .ppEnabledExtensionNames{ extension_names.data() }
         });
      }
   }

   vk::DebugUtilsMessengerEXT application::create_debug_callback_messenger() const
   {
      // QUESTION: it is required to dynamically load the function
      // that create the debug utils messenger since it's an
      // extension (reason why I'm passing dispatch_loader_dynamic_ here);
      // how come it's not needed in other extension functions?
      // ANSWER: that's the way it is, there are libraries that handle this
      // for you (like VOLK)
      return instance_.createDebugUtilsMessengerEXT({
         .messageSeverity{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
         },
         .messageType{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
         },
         .pfnUserCallback{
            [](VkDebugUtilsMessageSeverityFlagBitsEXT const,
            VkDebugUtilsMessageTypeFlagsEXT const,
            VkDebugUtilsMessengerCallbackDataEXT const* const callback_data,
            void* const) -> vk::Bool32
            {
               std::cout << std::format("[VALIDATION LAYER MESSAGE]\n{}\n\n", callback_data->pMessage);
               return false;
            }
         }
      }, nullptr, dispatch_loader_dynamic_);
   }

   vk::SurfaceKHR application::create_surface() const
   {
      if (VkSurfaceKHR surface; SDL_Vulkan_CreateSurface(window_.get(), instance_, nullptr, &surface))
         return surface;

      throw std::runtime_error("failed to create window surface!");
   }

   vk::PhysicalDevice application::pick_physical_device() const
   {
      std::vector const physical_devices{ instance_.enumeratePhysicalDevices() };
      if (physical_devices.empty())
         throw std::runtime_error("no physical device with Vulkan support found!");

      for (vk::PhysicalDevice const physical_device : physical_devices)
         if (physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            return physical_device;

      return physical_devices.front();
   }

   std::uint32_t application::graphics_queue_family_index() const
   {
      auto const queue_family_properties{ physical_device_.getQueueFamilyProperties() };
      for (std::uint32_t queue_family_index{}; queue_family_index < queue_family_properties.size(); ++queue_family_index)
         if (queue_family_properties[queue_family_index].queueFlags & vk::QueueFlagBits::eGraphics)
            return queue_family_index;

      throw std::runtime_error("no queue with support for graphics operations present!");
   }

   std::uint32_t application::presentation_queue_family_index() const
   {
      auto const queue_family_properties{ physical_device_.getQueueFamilyProperties() };
      for (std::uint32_t queue_family_index{}; queue_family_index < queue_family_properties.size(); ++queue_family_index)
         if (physical_device_.getSurfaceSupportKHR(queue_family_index, surface_))
            return queue_family_index;

      throw std::runtime_error("no queue with support for surface presentation present!");
   }

   vk::Device application::create_device() const
   {
      std::array constexpr queue_priorities{ 1.0f };

      std::vector<vk::DeviceQueueCreateInfo> queue_infos{
         {
            .queueFamilyIndex{ graphics_queue_family_index_ },
            .queueCount{ 1 },
            .pQueuePriorities{ queue_priorities.data() }
         },
         {
            .queueFamilyIndex{ presentation_queue_family_index_ },
            .queueCount{ 1 },
            .pQueuePriorities{ queue_priorities.data() }
         }
      };

      // QUESTION: the queueFamilyIndex member of each element of pQueueCreateInfos must be unique within pQueueCreateInfos,
      // except that two members can share the same queueFamilyIndex if one describes protected-capable queues and one describes
      // queues that are not protected-capable (graphics_queue_family_index_ and presentation_queue_family_index_ are the same
      // on my machines)
      // ANSWER: it probably has to do with memory access
      std::ranges::sort(queue_infos);
      auto const& [new_end, old_end]{ std::ranges::unique(queue_infos) };
      queue_infos.erase(new_end, old_end);

      std::array constexpr extension_names{ vk::KHRSwapchainExtensionName };

      vk::PhysicalDeviceFeatures constexpr device_features{
         .fillModeNonSolid{ true }
      };

      // TODO: for backwards compatibility, enable the same
      // validation layers on each logical device as
      // the ones enabled on the instance from which the
      // logical device is created
      return physical_device_.createDevice({
         .queueCreateInfoCount{ static_cast<std::uint32_t>(queue_infos.size()) },
         .pQueueCreateInfos{ queue_infos.data() },
         .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
         .ppEnabledExtensionNames{ extension_names.data() },
         .pEnabledFeatures{ &device_features }
      });
   }

   vk::SurfaceFormatKHR application::pick_swap_chain_format() const
   {
      std::vector const available_formats{ physical_device_.getSurfaceFormatsKHR(surface_) };

      if (available_formats.empty())
         throw std::runtime_error("no surface formats available!");

      for (vk::SurfaceFormatKHR const available_format : available_formats)
         if (available_format.format == vk::Format::eR8G8B8A8Srgb and
            available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return available_format;

      return available_formats.front();
   }

   vk::Extent2D application::pick_swap_chain_extent() const
   {
      if (vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };
         surface_capabilities.currentExtent.width not_eq std::numeric_limits<std::uint32_t>::max())
         return surface_capabilities.currentExtent;
      else
      {
         // QUESTION: the freedom of choosing the images'
         // extent between minImageExtent and maxImageExtent
         // is given by the window manager, why?

         int width;
         int height;
         if (not SDL_GetWindowSizeInPixels(window_.get(), &width, &height))
            throw std::runtime_error(std::format("failed to get window size in pixels: {}\n", SDL_GetError()));

         return {
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
      auto present_mode{ vk::PresentModeKHR::eFifo };
      for (auto const available_present_mode : physical_device_.getSurfacePresentModesKHR(surface_))
         if (available_present_mode == vk::PresentModeKHR::eMailbox)
         {
            present_mode = available_present_mode;
            break;
         }

      // NOTE: determines whether an image is owned by one
      // queue with explicit ownership transfers (eExclusive)
      // or can be used across multiple queues without explicit
      // ownership transfers (eConcurrent)
      vk::SharingMode sharing_mode;
      std::array<std::uint32_t, 2> queue_family_indices{};

      if (graphics_queue_family_index_ not_eq presentation_queue_family_index_)
      {
         sharing_mode = vk::SharingMode::eConcurrent;

         queue_family_indices[0] = graphics_queue_family_index_;
         queue_family_indices[1] = presentation_queue_family_index_;
      }
      else
         sharing_mode = vk::SharingMode::eExclusive;

      vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };

      return device_.createSwapchainKHR({
         .surface{ surface_ },
         // QUESTION: why is it recommended to have at least one more image than the minimum?
         // ANSWER: wrong in tutorial lol
         .minImageCount{
            // NOTE: a maxImageCount of 0 means that there is no maximum
            not surface_capabilities.maxImageCount
               ? surface_capabilities.minImageCount
               : std::min(surface_capabilities.minImageCount, surface_capabilities.maxImageCount)
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
         image_views.push_back(
            device_.createImageView({
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
      // QUESTION: the tutorial states: "...it's possible to
      // combine multiple fragment shaders into a single shader
      // module and use different entry points to differentiate
      // between their behaviors."; how do you put multiple
      // shaders into a single shader module?
      // ANSWER: by combine they mean combine the bytecode of
      // both shaders into one and then create a shader module
      // from with it
      return device_.createShaderModule({
         .codeSize{ sizeof(std::uint32_t) * byte_code.size() },
         .pCode{ byte_code.data() }
      });
   }

   vk::RenderPass application::create_render_pass() const
   {
      vk::AttachmentDescription const color_attachment_description{
         .format{ swap_chain_format_.format },
         .samples{ vk::SampleCountFlagBits::e1 },
         .loadOp{ vk::AttachmentLoadOp::eClear },
         .storeOp{ vk::AttachmentStoreOp::eStore },
         .stencilLoadOp{ vk::AttachmentLoadOp::eDontCare },
         .stencilStoreOp{ vk::AttachmentStoreOp::eDontCare },
         .initialLayout{ vk::ImageLayout::eUndefined },
         .finalLayout{ vk::ImageLayout::ePresentSrcKHR }
      };

      vk::AttachmentReference constexpr color_attachment_reference{
         .attachment{ 0 },
         .layout{ vk::ImageLayout::eColorAttachmentOptimal }
      };

      // ReSharper disable once CppVariableCanBeMadeConstexpr
      // this cannot be constexpr
      vk::SubpassDescription const subpass_description{
         .pipelineBindPoint{ vk::PipelineBindPoint::eGraphics },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &color_attachment_reference }
      };

      vk::SubpassDependency constexpr dependency{
         .srcSubpass{ vk::SubpassExternal },
         .srcStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput },
         .dstStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput },
         .dstAccessMask{ vk::AccessFlagBits::eColorAttachmentWrite }
      };

      return device_.createRenderPass({
         .attachmentCount{ 1 },
         .pAttachments{ &color_attachment_description },
         .subpassCount{ 1 },
         .pSubpasses{ &subpass_description },
         .dependencyCount{ 1 },
         .pDependencies{ &dependency }
      });
   }

   std::vector<vk::Framebuffer> application::create_frame_buffers() const
   {
      std::vector<vk::Framebuffer> framebuffers{};
      framebuffers.reserve(swap_chain_image_views_.size());

      for (vk::ImageView const image_view : swap_chain_image_views_)
         framebuffers.push_back(
            device_.createFramebuffer({
               .renderPass{ render_pass_ },
               .attachmentCount{ 1 },
               .pAttachments{ &image_view },
               .width{ swap_chain_extent_.width },
               .height{ swap_chain_extent_.height },
               .layers{ 1 }
            }));

      return framebuffers;
   }

   vk::PipelineLayout application::create_pipeline_layout() const
   {
      return device_.createPipelineLayout({});
   }

   vk::Pipeline application::create_pipeline() const
   {
      std::array constexpr dynamic_states{
         vk::DynamicState::eViewport,
         vk::DynamicState::eScissor
      };

      // ReSharper disable once CppVariableCanBeMadeConstexpr
      // this cannot be constexpr
      vk::PipelineDynamicStateCreateInfo const dynamic_state_create_info{
         .dynamicStateCount{ static_cast<std::uint32_t>(dynamic_states.size()) },
         .pDynamicStates{ dynamic_states.data() }
      };

      vk::ShaderModule const vertex_shader_module{
         create_shader_module(compile_shader("resources/shaders/shader.vert"))
      };
      vk::ShaderModule const fragment_shader_module{
         create_shader_module(compile_shader("resources/shaders/shader.frag"))
      };

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

      vk::PipelineVertexInputStateCreateInfo constexpr vertex_input_state_create_info{};

      // QUESTION: the type of primitives that will be generated
      // is specified here and if it's set to one of the line options, it will
      // not matter if the polygon mode in rasterization state is set to fill
      // since the assembled primitives are lines, however setting the polygon
      // mode to point with the topology set to line does not render points but
      // lines, why?
      vk::PipelineInputAssemblyStateCreateInfo constexpr input_assembly_state_create_info{
         .topology{ vk::PrimitiveTopology::eLineStrip }
      };

      vk::PipelineViewportStateCreateInfo constexpr viewport_state_create_info{
         .viewportCount{ 1 },
         .scissorCount{ 1 }
      };

      vk::PipelineRasterizationStateCreateInfo constexpr rasterization_state_create_info{
         .polygonMode{ vk::PolygonMode::ePoint },
         .cullMode{ vk::CullModeFlagBits::eBack },
         .frontFace{ vk::FrontFace::eClockwise },
         .lineWidth{ 1.0f }
      };

      vk::PipelineMultisampleStateCreateInfo constexpr multisample_state_create_info{
         .rasterizationSamples{ vk::SampleCountFlagBits::e1 }
      };

      vk::PipelineDepthStencilStateCreateInfo constexpr depth_stencil_state_create_info{};

      vk::PipelineColorBlendAttachmentState constexpr color_blend_attachment_state{
         .blendEnable{ true },
         .srcColorBlendFactor{ vk::BlendFactor::eSrcAlpha },
         .dstColorBlendFactor{ vk::BlendFactor::eOneMinusSrcAlpha },
         .colorBlendOp{ vk::BlendOp::eAdd },
         .srcAlphaBlendFactor{ vk::BlendFactor::eOne },
         .dstAlphaBlendFactor{ vk::BlendFactor::eZero },
         .alphaBlendOp{ vk::BlendOp::eAdd },
         .colorWriteMask{
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
         }
      };

      // ReSharper disable once CppVariableCanBeMadeConstexpr
      // this cannot be constexpr
      vk::PipelineColorBlendStateCreateInfo const color_blend_state_create_info{
         .attachmentCount{ 1 },
         .pAttachments{ &color_blend_attachment_state }
      };

      auto&& [_, pipeline]{
         device_.createGraphicsPipeline(nullptr, {
            .stageCount{ static_cast<std::uint32_t>(shader_stage_create_infos.size()) },
            .pStages{ shader_stage_create_infos.data() },
            .pVertexInputState{ &vertex_input_state_create_info },
            .pInputAssemblyState{ &input_assembly_state_create_info },
            .pViewportState{ &viewport_state_create_info },
            .pRasterizationState{ &rasterization_state_create_info },
            .pMultisampleState{ &multisample_state_create_info },
            .pDepthStencilState{ &depth_stencil_state_create_info },
            .pColorBlendState{ &color_blend_state_create_info },
            .pDynamicState{ &dynamic_state_create_info },
            .layout{ pipeline_layout_ },
            .renderPass{ render_pass_ }
         })
      };

      device_.destroyShaderModule(fragment_shader_module);
      device_.destroyShaderModule(vertex_shader_module);

      return pipeline;
   }

   vk::CommandPool application::create_command_pool() const
   {
      return device_.createCommandPool({
         .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
         .queueFamilyIndex{ graphics_queue_family_index_ }
      });
   }

   std::vector<vk::CommandBuffer> application::create_command_buffers() const
   {
      return device_.allocateCommandBuffers({
         .commandPool{ command_pool_ },
         .level{ vk::CommandBufferLevel::ePrimary },
         .commandBufferCount{ FRAMES_IN_FLIGHT },
      });
   }

   std::vector<vk::Semaphore> application::create_semaphores() const
   {
      std::vector<vk::Semaphore> semaphores(FRAMES_IN_FLIGHT);
      std::ranges::generate(semaphores, [this]
      {
         return device_.createSemaphore({});
      });

      return semaphores;
   }

   std::vector<vk::Fence> application::create_fences() const
   {
      std::vector<vk::Fence> fences(FRAMES_IN_FLIGHT);
      std::ranges::generate(fences, [this]
      {
         return device_.createFence({
            .flags{ vk::FenceCreateFlagBits::eSignaled }
         });
      });

      return fences;
   }

   void application::record_command_buffer(vk::CommandBuffer const command_buffer, std::uint32_t const image_index) const
   {
      vk::CommandBufferBeginInfo constexpr begin_info{};
      if (command_buffer.begin(&begin_info) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to begin recording command buffer!");

      vk::ClearValue constexpr clear_color_value{ { 0.0f, 0.0f, 0.0f, 1.0f } };
      command_buffer.beginRenderPass({
         .renderPass{ render_pass_ },
         .framebuffer{ swap_chain_framebuffers_[image_index] },
         .renderArea{
            .extent{ swap_chain_extent_ }
         },
         .clearValueCount{ 1 },
         .pClearValues{ &clear_color_value }
      }, vk::SubpassContents::eInline);

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

      vk::Viewport const viewport{
         .width{ static_cast<float>(swap_chain_extent_.width) },
         .height{ static_cast<float>(swap_chain_extent_.height) },
         .maxDepth{ 1.0f }
      };
      command_buffer.setViewport(0, 1, &viewport);

      vk::Rect2D const scissor{
         .extent{ swap_chain_extent_ }
      };
      command_buffer.setScissor(0, 1, &scissor);

      command_buffer.draw(13, 1, 0, 0);

      command_buffer.endRenderPass();

      command_buffer.end();
   }

   void application::draw_frame()
   {
      if (device_.waitForFences(1, &command_buffer_executed_fences_[current_frame_], true,
         std::numeric_limits<std::uint64_t>::max()) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to wait for fences!");

      auto&& [result, image_index]{
         device_.acquireNextImageKHR(swap_chain_, std::numeric_limits<std::uint64_t>::max(),
            image_available_semaphores_[current_frame_])
      };

      if (device_.resetFences(1, &command_buffer_executed_fences_[current_frame_]) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to reset fence!");

      command_buffers_[current_frame_].reset();
      record_command_buffer(command_buffers_[current_frame_], image_index);

      std::array<vk::PipelineStageFlags, 1> constexpr wait_stages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
      graphics_queue_.submit(vk::SubmitInfo{
         .waitSemaphoreCount{ 1 },
         .pWaitSemaphores{ &image_available_semaphores_[current_frame_] },
         .pWaitDstStageMask{ wait_stages.data() },
         .commandBufferCount{ 1 },
         .pCommandBuffers{ &command_buffers_[current_frame_] },
         .signalSemaphoreCount{ 1 },
         .pSignalSemaphores{ &render_finished_semaphores_[current_frame_] },
      }, command_buffer_executed_fences_[current_frame_]);

      if (presentation_queue_.presentKHR({
         .waitSemaphoreCount{ 1 },
         .pWaitSemaphores{ &render_finished_semaphores_[current_frame_] },
         .swapchainCount{ 1 },
         .pSwapchains{ &swap_chain_ },
         .pImageIndices{ &image_index }
      }) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to present!");

      current_frame_ = (current_frame_ + 1) % FRAMES_IN_FLIGHT;
   }

   void application::recreate_swapchain()
   {
      device_.waitIdle();

      for (vk::Framebuffer const framebuffer : swap_chain_framebuffers_)
         device_.destroyFramebuffer(framebuffer);

      for (vk::ImageView const image_view : swap_chain_image_views_)
         device_.destroyImageView(image_view);

      device_.destroySwapchainKHR(swap_chain_);

      swap_chain_format_ = pick_swap_chain_format();

      SDL_Event event;
      do
      {
         swap_chain_extent_ = pick_swap_chain_extent();
         SDL_WaitEvent(&event);
      }
      while (not swap_chain_extent_.width or not swap_chain_extent_.height);

      swap_chain_ = create_swap_chain();
      swap_chain_images_ = device_.getSwapchainImagesKHR(swap_chain_);
      swap_chain_image_views_ = create_image_views();
      swap_chain_framebuffers_ = create_frame_buffers();
   }
}