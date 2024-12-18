#include "erupch.hpp"

#include "application.hpp"
#include "shader_compiler/shader_compiler.hpp"

namespace eru
{
   application::~application()
   {
      device_.destroyFence(command_buffer_executed_fence);
      device_.destroySemaphore(render_finished_semaphore_);
      device_.destroySemaphore(image_available_semaphore_);
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

      instance_.destroy();
   }

   void application::run() const
   {
      while (not glfwWindowShouldClose(window_.get()))
      {
         glfwPollEvents();
         draw_frame();
      }

      device_.waitIdle();
   }

   vk::Instance application::create_instance()
   {
      #ifdef NDEBUG
      std::array<char const*, 0> constexpr validation_layer_names{};
      #else
      std::array constexpr validation_layer_names{ "VK_LAYER_KHRONOS_validation" };
      #endif

      std::uint32_t extension_count;
      char const* const* const extension_names{ glfwGetRequiredInstanceExtensions(&extension_count) };

      return vk::createInstance(
         {
            .enabledLayerCount{ static_cast<std::uint32_t>(validation_layer_names.size()) },
            .ppEnabledLayerNames{ validation_layer_names.data() },
            .enabledExtensionCount{ extension_count },
            .ppEnabledExtensionNames{ extension_names }
         });
   }

   vk::SurfaceKHR application::create_surface() const
   {
      if (VkSurfaceKHR surface; glfwCreateWindowSurface(instance_, window_.get(), nullptr, &surface) == VK_SUCCESS)
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

   std::uint32_t application::graphics_queue_index() const
   {
      std::uint32_t queue_index{};
      for (vk::QueueFamilyProperties const& queue : physical_device_.getQueueFamilyProperties())
      {
         if (queue.queueFlags & vk::QueueFlagBits::eGraphics)
            return queue_index;

         ++queue_index;
      }

      throw std::runtime_error("no queue with support for graphics operations present!");
   }

   std::uint32_t application::presentation_queue_index() const
   {
      for (std::uint32_t queue_index{}; queue_index < physical_device_.getQueueFamilyProperties().size(); ++queue_index)
         if (physical_device_.getSurfaceSupportKHR(queue_index, surface_))
            return queue_index;

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
      auto const& [new_end, old_end]{ std::ranges::unique(queue_infos) };
      queue_infos.erase(new_end, old_end);

      std::array constexpr extension_names{ vk::KHRSwapchainExtensionName };

      // TODO: for backwards compatibility, enable the same
      // validation layers on each logical device as
      // the ones enabled on the instance from which the
      // logical device is created
      return physical_device_.createDevice(
         {
            .queueCreateInfoCount{ static_cast<std::uint32_t>(queue_infos.size()) },
            .pQueueCreateInfos{ queue_infos.data() },
            .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
            .ppEnabledExtensionNames{ extension_names.data() }
         });
   }

   vk::SurfaceFormatKHR application::pick_swap_chain_format() const
   {
      std::vector const available_formats{ physical_device_.getSurfaceFormatsKHR(surface_) };
      for (vk::SurfaceFormatKHR const available_format : available_formats)
         if (available_format.format == vk::Format::eB8G8R8A8Srgb and
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
         int width;
         int height;
         glfwGetFramebufferSize(window_.get(), &width, &height);

         return {
            .width{
               std::clamp(
                  static_cast<std::uint32_t>(width),
                  surface_capabilities.minImageExtent.width,
                  surface_capabilities.maxImageExtent.width)
            },
            .height{
               std::clamp(
                  static_cast<std::uint32_t>(height),
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

      vk::SharingMode sharing_mode;
      std::vector<std::uint32_t> queue_family_indices{};

      // ReSharper disable once CppDFAConstantConditions
      // how is this always true? ._.
      if (graphics_queue_index_ not_eq presentation_queue_index_)
      {
         sharing_mode = vk::SharingMode::eConcurrent;

         queue_family_indices.resize(2);
         queue_family_indices[0] = graphics_queue_index_;
         queue_family_indices[1] = presentation_queue_index_;
      } else
         sharing_mode = vk::SharingMode::eExclusive;

      vk::SurfaceCapabilitiesKHR const surface_capabilities{ physical_device_.getSurfaceCapabilitiesKHR(surface_) };
      return device_.createSwapchainKHR(
         {
            .surface{ surface_ },
            .minImageCount{
               surface_capabilities.maxImageCount not_eq 0
               ? std::min(surface_capabilities.minImageCount + 1, surface_capabilities.maxImageCount)
               : surface_capabilities.minImageCount + 1
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
            device_.createImageView(
               {
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
      return device_.createShaderModule(
         {
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

      return device_.createRenderPass(
         {
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
            device_.createFramebuffer(
               {
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
      std::array constexpr dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

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

      vk::PipelineInputAssemblyStateCreateInfo constexpr input_assembly_state_create_info{
         .topology{ vk::PrimitiveTopology::eTriangleList },
         .primitiveRestartEnable{ false }
      };

      vk::PipelineViewportStateCreateInfo constexpr viewport_state_create_info{
         .viewportCount{ 1 },
         .scissorCount{ 1 }
      };

      vk::PipelineRasterizationStateCreateInfo constexpr rasterization_state_create_info{
         .depthClampEnable{ false },
         .rasterizerDiscardEnable{ false },
         .polygonMode{ vk::PolygonMode::eFill },
         .cullMode{ vk::CullModeFlagBits::eBack },
         .frontFace{ vk::FrontFace::eClockwise },
         .depthBiasEnable{ false },
         .lineWidth{ 1.0f }
      };

      vk::PipelineMultisampleStateCreateInfo constexpr multisample_state_create_info{
         .rasterizationSamples{ vk::SampleCountFlagBits::e1 },
         .sampleShadingEnable{ false }
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
         .logicOpEnable{ false },
         .logicOp{ vk::LogicOp::eCopy },
         .attachmentCount{ 1 },
         .pAttachments{ &color_blend_attachment_state }
      };

      auto&& [result, pipeline]{
         device_.createGraphicsPipeline(
            nullptr, {
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
      return device_.createCommandPool(
         {
            .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
            .queueFamilyIndex{ graphics_queue_index_ }
         });
   }

   vk::CommandBuffer application::create_command_buffer() const
   {
      return device_.allocateCommandBuffers(
         {
            .commandPool{ command_pool_ },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ 1 }
         }).front();
   }

   void application::record_command_buffer(vk::CommandBuffer const command_buffer, std::uint32_t const image_index) const
   {
      vk::CommandBufferBeginInfo constexpr begin_info{};
      if (command_buffer.begin(&begin_info) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to begin recording command buffer!");

      vk::ClearValue constexpr clear_color_value{ { 0.0f, 0.0f, 0.0f, 1.0f } };
      command_buffer.beginRenderPass(
         {
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

      command_buffer.draw(3, 1, 0, 0);

      command_buffer.endRenderPass();

      command_buffer.end();
   }

   void application::draw_frame() const
   {
      if (device_.waitForFences(1, &command_buffer_executed_fence, true, std::numeric_limits<std::uint64_t>::max()) not_eq
         vk::Result::eSuccess)
         throw std::runtime_error("failed to wait for fences!");

      if (device_.resetFences(1, &command_buffer_executed_fence) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to reset fence!");

      auto&& [result, image_index]{
         device_.acquireNextImageKHR(swap_chain_, std::numeric_limits<std::uint64_t>::max(), image_available_semaphore_)
      };

      command_buffer_.reset();
      record_command_buffer(command_buffer_, image_index);

      std::array<vk::PipelineStageFlags, 1> constexpr wait_stages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
      graphics_queue_.submit(
         vk::SubmitInfo{
            .waitSemaphoreCount{ 1 },
            .pWaitSemaphores{ &image_available_semaphore_ },
            .pWaitDstStageMask{ wait_stages.data() },
            .commandBufferCount{ 1 },
            .pCommandBuffers{ &command_buffer_ },
            .signalSemaphoreCount{ 1 },
            .pSignalSemaphores{ &render_finished_semaphore_ },
         }, command_buffer_executed_fence);

      if (presentation_queue_.presentKHR(
         {
            .waitSemaphoreCount{ 1 },
            .pWaitSemaphores{ &render_finished_semaphore_ },
            .swapchainCount{ 1 },
            .pSwapchains{ &swap_chain_ },
            .pImageIndices{ &image_index }
         }) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to present!");
   }
}