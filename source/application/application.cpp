#include "application.hpp"
#include "math/uniform_buffer_object.hpp"
#include "shader_compiler/shader_compiler.hpp"

namespace eru
{
   Application::~Application()
   {
      for (vk::Fence const fence : command_buffer_executed_fences_)
         device_.destroyFence(fence);

      for (vk::Semaphore const semaphore : render_finished_semaphores_)
         device_.destroySemaphore(semaphore);

      for (vk::Semaphore const semaphore : image_available_semaphores_)
         device_.destroySemaphore(semaphore);

      device_.destroyCommandPool(command_pool_);

      device_.destroyDescriptorPool(descriptor_pool_);
      for (auto&& [buffer, allocation] : uniform_buffers_)
         vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer), allocation);

      vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(index_buffer_.first), index_buffer_.second);
      vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(vertex_buffer_.first), vertex_buffer_.second);
      device_.destroySampler(texture_sampler_);
      device_.destroyImageView(texture_image_view_);
      vmaDestroyImage(allocator_, static_cast<VkImage>(texture_image_.first), texture_image_.second);
      device_.destroyImageView(depth_image_view_);
      vmaDestroyImage(allocator_, static_cast<VkImage>(depth_image_.first), depth_image_.second);
      vmaDestroyAllocator(allocator_);

      device_.destroyPipeline(pipeline_);
      device_.destroyPipelineLayout(pipeline_layout_);
      device_.destroyDescriptorSetLayout(descriptor_set_layout_);
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

   void Application::run()
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

   vk::Instance Application::create_instance()
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

   vk::DebugUtilsMessengerEXT Application::create_debug_callback_messenger() const
   {
      // QUESTION: it is required to dynamically load the function
      // that create the debug utils messenger since it's an
      // extension (reason why I'm passing dispatch_loader_dynamic_ here);
      // how come it's not needed in other extension functions?
      // ANSWER: that's the way it is, there are libraries that handle this
      // for you (like VOLK)
      return instance_.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT{
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
            [](VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
            VkDebugUtilsMessengerCallbackDataEXT const* callback_data, void*) -> VkBool32
            {
               std::cout << std::format("[VALIDATION LAYER MESSAGE]\n{}\n\n", callback_data->pMessage);
               return false;
            }
         }
      }, nullptr, dispatch_loader_dynamic_);
   }

   vk::SurfaceKHR Application::create_surface() const
   {
      if (VkSurfaceKHR surface; SDL_Vulkan_CreateSurface(window_.get(), instance_, nullptr, &surface))
         return static_cast<vk::SurfaceKHR>(surface);

      throw std::runtime_error("failed to create window surface!");
   }

   vk::PhysicalDevice Application::pick_physical_device() const
   {
      std::vector const physical_devices{ instance_.enumeratePhysicalDevices() };
      if (physical_devices.empty())
         throw std::runtime_error("no physical device with Vulkan support found!");

      for (vk::PhysicalDevice const physical_device : physical_devices)
         if (physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            return physical_device;

      return physical_devices.front();
   }

   std::uint32_t Application::graphics_queue_family_index() const
   {
      auto const queue_family_properties{ physical_device_.getQueueFamilyProperties() };
      for (std::uint32_t queue_family_index{}; queue_family_index < queue_family_properties.size(); ++queue_family_index)
         if (queue_family_properties[queue_family_index].queueFlags & vk::QueueFlagBits::eGraphics)
            return queue_family_index;

      throw std::runtime_error("no queue with support for graphics operations present!");
   }

   std::uint32_t Application::presentation_queue_family_index() const
   {
      auto const queue_family_properties{ physical_device_.getQueueFamilyProperties() };
      for (std::uint32_t queue_family_index{}; queue_family_index < queue_family_properties.size(); ++queue_family_index)
         if (physical_device_.getSurfaceSupportKHR(queue_family_index, surface_))
            return queue_family_index;

      throw std::runtime_error("no queue with support for surface presentation present!");
   }

   vk::Device Application::create_device() const
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
         .fillModeNonSolid{ true },
         .samplerAnisotropy{ true }
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

   vk::SurfaceFormatKHR Application::pick_swap_chain_format() const
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

   vk::Extent2D Application::pick_swap_chain_extent() const
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

   vk::SwapchainKHR Application::create_swap_chain() const
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

   std::vector<vk::ImageView> Application::create_image_views() const
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

   vk::ShaderModule Application::create_shader_module(std::vector<std::uint32_t> const& byte_code) const
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

   vk::RenderPass Application::create_render_pass() const
   {
      std::array<vk::AttachmentDescription, 2> const attachment_descriptions{
         {
            {
               .format{ swap_chain_format_.format },
               .samples{ vk::SampleCountFlagBits::e1 },
               .loadOp{ vk::AttachmentLoadOp::eClear },
               .storeOp{ vk::AttachmentStoreOp::eStore },
               .stencilLoadOp{ vk::AttachmentLoadOp::eDontCare },
               .stencilStoreOp{ vk::AttachmentStoreOp::eDontCare },
               .initialLayout{ vk::ImageLayout::eUndefined },
               .finalLayout{ vk::ImageLayout::ePresentSrcKHR }
            },
            {
               .format{ vk::Format::eD32Sfloat },
               .samples{ vk::SampleCountFlagBits::e1 },
               .loadOp{ vk::AttachmentLoadOp::eClear },
               .storeOp{ vk::AttachmentStoreOp::eDontCare },
               .stencilLoadOp{ vk::AttachmentLoadOp::eDontCare },
               .stencilStoreOp{ vk::AttachmentStoreOp::eDontCare },
               .initialLayout{ vk::ImageLayout::eUndefined },
               .finalLayout{ vk::ImageLayout::eDepthStencilAttachmentOptimal }
            }
         }
      };

      std::array<vk::AttachmentReference, 2> constexpr attachment_references{
         {
            {
               .attachment{ 0 },
               .layout{ vk::ImageLayout::eColorAttachmentOptimal }
            },
            {
               .attachment{ 1 },
               .layout{ vk::ImageLayout::eDepthStencilAttachmentOptimal }
            }
         }
      };

      vk::SubpassDescription const subpass_description{
         .pipelineBindPoint{ vk::PipelineBindPoint::eGraphics },
         .colorAttachmentCount{ 1 },
         .pColorAttachments{ &attachment_references[0] },
         .pDepthStencilAttachment{ &attachment_references[1] }
      };

      vk::SubpassDependency constexpr dependency{
         .srcSubpass{ vk::SubpassExternal },
         .dstSubpass{ 0 },
         .srcStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests },
         .dstStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests },
         .srcAccessMask{ vk::AccessFlagBits::eNone },
         .dstAccessMask{ vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite },
      };

      return device_.createRenderPass({
         .attachmentCount{ static_cast<std::uint32_t>(attachment_descriptions.size()) },
         .pAttachments{ attachment_descriptions.data() },
         .subpassCount{ 1 },
         .pSubpasses{ &subpass_description },
         .dependencyCount{ 1 },
         .pDependencies{ &dependency }
      });
   }

   std::vector<vk::Framebuffer> Application::create_frame_buffers() const
   {
      std::vector<vk::Framebuffer> framebuffers{};
      framebuffers.reserve(swap_chain_image_views_.size());

      for (vk::ImageView const image_view : swap_chain_image_views_)
      {
         std::array const attachments{ image_view, depth_image_view_ };

         framebuffers.push_back(
            device_.createFramebuffer({
               .renderPass{ render_pass_ },
               .attachmentCount{ static_cast<std::uint32_t>(attachments.size()) },
               .pAttachments{ attachments.data() },
               .width{ swap_chain_extent_.width },
               .height{ swap_chain_extent_.height },
               .layers{ 1 }
            }));
      }

      return framebuffers;
   }

   vk::DescriptorSetLayout Application::create_descriptor_set_layout() const
   {
      std::array<vk::DescriptorSetLayoutBinding, 2> constexpr descriptor_set_layout_bindings{
         {
            {
               .binding{ 0 },
               .descriptorType{ vk::DescriptorType::eUniformBuffer },
               .descriptorCount{ 1 },
               .stageFlags{ vk::ShaderStageFlagBits::eVertex },
            },
            {
               .binding{ 1 },
               .descriptorType{ vk::DescriptorType::eCombinedImageSampler },
               .descriptorCount{ 1 },
               .stageFlags{ vk::ShaderStageFlagBits::eFragment },
            }
         }
      };

      return device_.createDescriptorSetLayout({
         .bindingCount{ static_cast<std::uint32_t>(descriptor_set_layout_bindings.size()) },
         .pBindings{ descriptor_set_layout_bindings.data() }
      });
   }

   vk::PipelineLayout Application::create_pipeline_layout() const
   {
      return device_.createPipelineLayout({
         .setLayoutCount{ 1 },
         .pSetLayouts{ &descriptor_set_layout_ },
      });
   }

   vk::Pipeline Application::create_pipeline() const
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

      vk::PipelineVertexInputStateCreateInfo constexpr vertex_input_state_create_info{
         .vertexBindingDescriptionCount{ static_cast<std::uint32_t>(Vertex::BINDING_DESCRIPTIONS.size()) },
         .pVertexBindingDescriptions{ Vertex::BINDING_DESCRIPTIONS.data() },
         .vertexAttributeDescriptionCount{ static_cast<std::uint32_t>(Vertex::ATTRIBUTE_DESCRIPTIONS.size()) },
         .pVertexAttributeDescriptions{ Vertex::ATTRIBUTE_DESCRIPTIONS.data() }
      };

      // QUESTION: the type of primitives that will be generated
      // is specified here and if it's set to one of the line options, it will
      // not matter if the polygon mode in rasterization state is set to fill
      // since the assembled primitives are lines, however setting the polygon
      // mode to point with the topology set to line does not render points but
      // lines, why?
      vk::PipelineInputAssemblyStateCreateInfo constexpr input_assembly_state_create_info{
         .topology{ vk::PrimitiveTopology::eTriangleList },
      };

      vk::PipelineViewportStateCreateInfo constexpr viewport_state_create_info{
         .viewportCount{ 1 },
         .scissorCount{ 1 }
      };

      vk::PipelineRasterizationStateCreateInfo constexpr rasterization_state_create_info{
         .polygonMode{ vk::PolygonMode::eFill },
         .cullMode{ vk::CullModeFlagBits::eBack },
         .frontFace{ vk::FrontFace::eClockwise },
         .lineWidth{ 1.0f }
      };

      vk::PipelineMultisampleStateCreateInfo constexpr multisample_state_create_info{
         .rasterizationSamples{ vk::SampleCountFlagBits::e1 }
      };

      vk::PipelineDepthStencilStateCreateInfo constexpr depth_stencil_state_create_info{
         .depthTestEnable{ true },
         .depthWriteEnable{ true },
         .depthCompareOp{ vk::CompareOp::eLess },
      };

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

      auto&& [result, pipeline]{
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
            .renderPass{ render_pass_ },
            .subpass{ 0 }
         })
      };

      device_.destroyShaderModule(fragment_shader_module);
      device_.destroyShaderModule(vertex_shader_module);

      return pipeline;
   }

   vk::CommandPool Application::create_command_pool() const
   {
      return device_.createCommandPool({
         .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
         .queueFamilyIndex{ graphics_queue_family_index_ }
      });
   }

   std::pair<std::vector<Vertex>, std::vector<std::uint32_t>> Application::load_model()
   {
      Assimp::Importer importer{};
      aiScene const* const scene{
         importer.ReadFile("resources/models/viking_room.obj",
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_GenNormals)
      };

      if (not scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or not scene->mRootNode)
         throw std::runtime_error("failed to load model!");

      std::vector<Vertex> vertices{};
      std::vector<std::uint32_t> indices{};
      for (aiMesh const* const mesh : std::vector<aiMesh*>{ scene->mMeshes, scene->mMeshes + scene->mNumMeshes })
      {
         if (mesh->mPrimitiveTypes not_eq aiPrimitiveType_TRIANGLE)
            throw std::runtime_error("model is not triangulated!");

         vertices.reserve(vertices.size() + mesh->mNumVertices);
         indices.reserve(indices.size() + mesh->mNumFaces * 3);

         for (unsigned int index{}; index < mesh->mNumVertices; ++index)
            vertices.push_back({
               .position{ mesh->mVertices[index].x, mesh->mVertices[index].z, mesh->mVertices[index].y },
               .color{ 1.0f, 1.0f, 1.0f },
               .uv{ mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y }
            });

         for (unsigned int index{}; index < mesh->mNumFaces; ++index)
         {
            aiFace const face{ mesh->mFaces[index] };
            indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
         }
      }

      return { vertices, indices };
   }

   VmaAllocator Application::create_allocator() const
   {
      VmaAllocatorCreateInfo const allocator_create_info{
         .flags{},
         .physicalDevice{ physical_device_ },
         .device{ device_ },
         .preferredLargeHeapBlockSize{},
         .pAllocationCallbacks{},
         .pDeviceMemoryCallbacks{},
         .pHeapSizeLimit{},
         .pVulkanFunctions{},
         .instance{ instance_ },
         .vulkanApiVersion{},
         .pTypeExternalMemoryHandleTypes{},
      };

      VmaAllocator allocator;
      if (vmaCreateAllocator(&allocator_create_info, &allocator) not_eq VK_SUCCESS)
         throw std::runtime_error("failed to create Vulkan memory allocator!");

      return allocator;
   }

   std::pair<vk::Buffer, VmaAllocation> Application::create_buffer(vk::DeviceSize const size, vk::BufferUsageFlags const usage,
      VmaAllocationCreateFlags const allocation_flags, vk::MemoryPropertyFlags const required_properties,
      vk::MemoryPropertyFlags const preferred_properties) const
   {
      vk::BufferCreateInfo const buffer_create_info{
         .size{ size },
         .usage{ usage },
         .sharingMode{ vk::SharingMode::eExclusive }
      };

      VmaAllocationCreateInfo const allocation_create_info{
         .flags{ allocation_flags },
         .usage{},
         .requiredFlags{ static_cast<VkMemoryPropertyFlags>(required_properties) },
         .preferredFlags{ static_cast<VkMemoryPropertyFlags>(preferred_properties) },
         .memoryTypeBits{},
         .pool{},
         .pUserData{},
         .priority{}
      };

      VkBuffer buffer;
      VmaAllocation memory;
      vmaCreateBuffer(allocator_, &static_cast<VkBufferCreateInfo const&>(buffer_create_info), &allocation_create_info,
         &buffer, &memory, nullptr);

      return { static_cast<vk::Buffer>(buffer), memory };
   }

   std::pair<vk::Image, VmaAllocation> Application::create_image(vk::Format const format, vk::ImageTiling const tiling,
      vk::Extent3D const extent, vk::ImageUsageFlags const usage, VmaAllocationCreateFlags const allocation_flags,
      vk::MemoryPropertyFlags const required_properties, vk::MemoryPropertyFlags const preferred_properties) const
   {
      vk::ImageCreateInfo const image_create_info{
         .imageType{ vk::ImageType::e2D },
         .format{ format },
         .extent{ extent },
         .mipLevels{ 1 },
         .arrayLayers{ 1 },
         .samples{ vk::SampleCountFlagBits::e1 },
         .tiling{ tiling },
         .usage{ usage },
         .sharingMode{ vk::SharingMode::eExclusive },
         .initialLayout{ vk::ImageLayout::eUndefined }
      };

      VmaAllocationCreateInfo const allocation_create_info{
         .flags{ allocation_flags },
         .usage{},
         .requiredFlags{ static_cast<VkMemoryPropertyFlags>(required_properties) },
         .preferredFlags{ static_cast<VkMemoryPropertyFlags>(preferred_properties) },
         .memoryTypeBits{},
         .pool{},
         .pUserData{},
         .priority{}
      };

      VkImage image;
      VmaAllocation memory;
      vmaCreateImage(allocator_, &static_cast<VkImageCreateInfo const&>(image_create_info), &allocation_create_info,
         &image, &memory, nullptr);

      return { static_cast<vk::Image>(image), memory };
   }

   void Application::copy_buffer(vk::Buffer const source_buffer, vk::Buffer const target_buffer, vk::DeviceSize size) const
   {
      vk::CommandBuffer const command_buffer{ begin_single_time_commands() };

      command_buffer.copyBuffer(source_buffer, target_buffer, {
         {
            .size{ size }
         }
      });

      end_single_time_commands(command_buffer);
   }

   void Application::copy_buffer(vk::Buffer const buffer, vk::Image const image, vk::Extent3D const extent) const
   {
      vk::CommandBuffer const command_buffer{ begin_single_time_commands() };

      command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, {
         {
            .imageSubresource{
               .aspectMask{ vk::ImageAspectFlagBits::eColor },
               .layerCount{ 1 }
            },
            .imageExtent{ extent }
         }
      });

      end_single_time_commands(command_buffer);
   }

   std::pair<vk::Image, VmaAllocation> Application::create_depth_image() const
   {
      return create_image(vk::Format::eD32Sfloat, vk::ImageTiling::eOptimal,
         {
            .width{ swap_chain_extent_.width },
            .height{ swap_chain_extent_.height },
            .depth{ 1 }
         },
         vk::ImageUsageFlagBits::eDepthStencilAttachment,
         {},
         vk::MemoryPropertyFlagBits::eDeviceLocal,
         {});
   }

   vk::ImageView Application::create_depth_image_view() const
   {
      return device_.createImageView({
         .image{ depth_image_.first },
         .viewType{ vk::ImageViewType::e2D },
         .format{ vk::Format::eD32Sfloat },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eDepth },
            .levelCount{ 1 },
            .layerCount{ 1 }
         }
      });
   }

   std::pair<vk::Image, VmaAllocation> Application::create_texture_image() const
   {
      std::filesystem::path const texture_path{ "resources/textures/viking_room.png" };
      if (not std::filesystem::exists(texture_path))
         throw std::runtime_error("failed to find image file!");

      UniquePointer<SDL_Surface> texture{ IMG_Load(texture_path.string().c_str()), SDL_DestroySurface };
      texture.reset(SDL_ConvertSurface(texture.get(), SDL_PIXELFORMAT_RGBA32));

      auto const image_size{ static_cast<vk::DeviceSize>(texture->pitch * texture->h) };
      vk::Extent3D const image_extent{
         .width{ static_cast<std::uint32_t>(texture->w) },
         .height{ static_cast<std::uint32_t>(texture->h) },
         .depth{ 1 }
      };

      auto const [staging_buffer, staging_allocation]{
         create_buffer(image_size,
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            {})
      };

      VmaAllocationInfo staging_allocation_info;
      vmaGetAllocationInfo(allocator_, staging_allocation, &staging_allocation_info);
      std::memcpy(staging_allocation_info.pMappedData, texture->pixels, static_cast<std::size_t>(image_size));

      std::pair const texture_image{
         create_image(vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, image_extent,
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            {},
            vk::MemoryPropertyFlagBits::eDeviceLocal, {})
      };

      transition_image_layout(texture_image.first, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
         vk::ImageLayout::eTransferDstOptimal);
      copy_buffer(staging_buffer, texture_image.first, image_extent);
      vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(staging_buffer), staging_allocation);

      transition_image_layout(texture_image.first, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal,
         vk::ImageLayout::eShaderReadOnlyOptimal);
      return texture_image;
   }

   vk::ImageView Application::create_texture_image_view() const
   {
      return device_.createImageView({
         .image{ texture_image_.first },
         .viewType{ vk::ImageViewType::e2D },
         .format{ vk::Format::eR8G8B8A8Srgb },
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
      });
   }

   vk::Sampler Application::create_texture_sampler() const
   {
      return device_.createSampler({
         .magFilter{ vk::Filter::eLinear },
         .minFilter{ vk::Filter::eLinear },
         .mipmapMode{ vk::SamplerMipmapMode::eLinear },
         .addressModeU{ vk::SamplerAddressMode::eRepeat },
         .addressModeV{ vk::SamplerAddressMode::eRepeat },
         .addressModeW{ vk::SamplerAddressMode::eRepeat },
         .anisotropyEnable{ true },
         .maxAnisotropy{ physical_device_.getProperties().limits.maxSamplerAnisotropy },
         .compareOp{ vk::CompareOp::eAlways },
         .borderColor{ vk::BorderColor::eIntOpaqueBlack },
      });
   }

   std::pair<vk::Buffer, VmaAllocation> Application::create_vertex_buffer() const
   {
      vk::DeviceSize const buffer_size{ sizeof(decltype(model_.first)::value_type) * model_.first.size() };

      auto const [staging_buffer, staging_allocation]{
         create_buffer(buffer_size,
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            vk::MemoryPropertyFlagBits::eDeviceLocal)
      };

      VmaAllocationInfo allocation_info;
      vmaGetAllocationInfo(allocator_, staging_allocation, &allocation_info);
      std::memcpy(allocation_info.pMappedData, model_.first.data(), static_cast<std::size_t>(buffer_size));

      std::pair const vertex_buffer{
         create_buffer(buffer_size,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
            {},
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            {})
      };

      copy_buffer(staging_buffer, vertex_buffer.first, buffer_size);

      vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(staging_buffer), staging_allocation);

      return vertex_buffer;
   }

   std::pair<vk::Buffer, VmaAllocation> Application::create_index_buffer() const
   {
      vk::DeviceSize const buffer_size{ sizeof(decltype(model_.second)::value_type) * model_.second.size() };

      auto const [staging_buffer, staging_allocation]{
         create_buffer(buffer_size,
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            vk::MemoryPropertyFlagBits::eDeviceLocal)
      };

      VmaAllocationInfo allocation_info;
      vmaGetAllocationInfo(allocator_, staging_allocation, &allocation_info);
      std::memcpy(allocation_info.pMappedData, model_.second.data(), static_cast<std::size_t>(buffer_size));

      std::pair const index_buffer{
         create_buffer(buffer_size,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
            {},
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            {})
      };

      copy_buffer(staging_buffer, index_buffer.first, buffer_size);

      vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(staging_buffer), staging_allocation);

      return index_buffer;
   }

   std::vector<std::pair<vk::Buffer, VmaAllocation>> Application::create_uniform_buffers() const
   {
      vk::DeviceSize constexpr buffer_size{ sizeof(UniformBufferObject) };

      std::vector<std::pair<vk::Buffer, VmaAllocation>> uniform_buffers(FRAMES_IN_FLIGHT);
      std::ranges::generate(uniform_buffers,
         [this, buffer_size]
         {
            return create_buffer(buffer_size,
               vk::BufferUsageFlagBits::eUniformBuffer,
               VMA_ALLOCATION_CREATE_MAPPED_BIT,
               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
               {});
         });

      return uniform_buffers;
   }

   vk::DescriptorPool Application::create_descriptor_pool() const
   {
      std::array<vk::DescriptorPoolSize, 2> constexpr descriptor_pool_sizes{
         {
            {
               .type{ vk::DescriptorType::eUniformBuffer },
               .descriptorCount{ FRAMES_IN_FLIGHT }
            },
            {
               .type{ vk::DescriptorType::eCombinedImageSampler },
               .descriptorCount{ FRAMES_IN_FLIGHT }
            }
         }
      };

      return device_.createDescriptorPool({
         .maxSets{ FRAMES_IN_FLIGHT },
         .poolSizeCount{ static_cast<std::uint32_t>(descriptor_pool_sizes.size()) },
         .pPoolSizes{ descriptor_pool_sizes.data() }
      });
   }

   std::vector<vk::DescriptorSet> Application::create_descriptor_sets() const
   {
      std::vector const descriptor_set_layouts{ FRAMES_IN_FLIGHT, descriptor_set_layout_ };
      std::vector const descriptor_sets{
         device_.allocateDescriptorSets({
            .descriptorPool{ descriptor_pool_ },
            .descriptorSetCount{ static_cast<std::uint32_t>(descriptor_set_layouts.size()) },
            .pSetLayouts{ descriptor_set_layouts.data() }
         })
      };

      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
      {
         vk::DescriptorBufferInfo const buffer_info{
            .buffer{ uniform_buffers_[index].first },
            .offset{ 0 },
            .range{ sizeof(UniformBufferObject) }
         };

         vk::DescriptorImageInfo const image_info{
            .sampler{ texture_sampler_ },
            .imageView{ texture_image_view_ },
            .imageLayout{ vk::ImageLayout::eShaderReadOnlyOptimal }
         };

         device_.updateDescriptorSets({
            {
               .dstSet{ descriptor_sets[index] },
               .dstBinding{ 0 },
               .dstArrayElement{ 0 },
               .descriptorCount{ 1 },
               .descriptorType{ vk::DescriptorType::eUniformBuffer },
               .pBufferInfo{ &buffer_info }
            },
            {
               .dstSet{ descriptor_sets[index] },
               .dstBinding{ 1 },
               .dstArrayElement{ 0 },
               .descriptorCount{ 1 },
               .descriptorType{ vk::DescriptorType::eCombinedImageSampler },
               .pImageInfo{ &image_info }
            }
         }, {});
      }

      return descriptor_sets;
   }

   std::vector<vk::CommandBuffer> Application::create_command_buffers() const
   {
      return device_.allocateCommandBuffers({
         .commandPool{ command_pool_ },
         .level{ vk::CommandBufferLevel::ePrimary },
         .commandBufferCount{ FRAMES_IN_FLIGHT },
      });
   }

   std::vector<vk::Semaphore> Application::create_semaphores() const
   {
      std::vector<vk::Semaphore> semaphores(FRAMES_IN_FLIGHT);
      std::ranges::generate(semaphores,
         [this]
         {
            return device_.createSemaphore({});
         });

      return semaphores;
   }

   std::vector<vk::Fence> Application::create_fences() const
   {
      std::vector<vk::Fence> fences(FRAMES_IN_FLIGHT);
      std::ranges::generate(fences,
         [this]
         {
            return device_.createFence({
               .flags{ vk::FenceCreateFlagBits::eSignaled }
            });
         });

      return fences;
   }

   vk::CommandBuffer Application::begin_single_time_commands() const
   {
      vk::CommandBuffer const command_buffer{
         device_.allocateCommandBuffers({
            .commandPool{ command_pool_ },
            .level{ vk::CommandBufferLevel::ePrimary },
            .commandBufferCount{ 1 }
         }).front()
      };

      command_buffer.begin({
         .flags{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }
      });

      return command_buffer;
   }

   void Application::end_single_time_commands(vk::CommandBuffer command_buffer) const
   {
      command_buffer.end();

      graphics_queue_.submit({
         {
            .commandBufferCount{ 1 },
            .pCommandBuffers{ &command_buffer }
         }
      });
      graphics_queue_.waitIdle();

      device_.freeCommandBuffers(command_pool_, 1, &command_buffer);
   }

   void Application::transition_image_layout(vk::Image image, vk::Format, vk::ImageLayout old_layout,
      vk::ImageLayout new_layout) const
   {
      vk::CommandBuffer const command_buffer{ begin_single_time_commands() };

      vk::AccessFlags source_access_mask;
      vk::AccessFlags destination_access_mask;
      vk::PipelineStageFlags source_stage_mask;
      vk::PipelineStageFlags destination_stage_mask;
      if (old_layout == vk::ImageLayout::eUndefined and new_layout == vk::ImageLayout::eTransferDstOptimal)
      {
         source_access_mask = vk::AccessFlagBits::eNone;
         destination_access_mask = vk::AccessFlagBits::eTransferWrite;

         source_stage_mask = vk::PipelineStageFlagBits::eTopOfPipe;
         destination_stage_mask = vk::PipelineStageFlagBits::eTransfer;
      }
      else if (old_layout == vk::ImageLayout::eTransferDstOptimal and new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
      {
         source_access_mask = vk::AccessFlagBits::eTransferWrite;
         destination_access_mask = vk::AccessFlagBits::eShaderRead;

         source_stage_mask = vk::PipelineStageFlagBits::eTransfer;
         destination_stage_mask = vk::PipelineStageFlagBits::eFragmentShader;
      }
      else
         throw std::invalid_argument("unsupported layout transition!");

      vk::ImageMemoryBarrier const image_memory_barrier{
         .srcAccessMask{ source_access_mask },
         .dstAccessMask{ destination_access_mask },
         .oldLayout{ old_layout },
         .newLayout{ new_layout },
         .srcQueueFamilyIndex{ vk::QueueFamilyIgnored },
         .dstQueueFamilyIndex{ vk::QueueFamilyIgnored },
         .image{ image },
         .subresourceRange{
            .aspectMask{ vk::ImageAspectFlagBits::eColor },
            .baseMipLevel{ 0 },
            .levelCount{ 1 },
            .baseArrayLayer{ 0 },
            .layerCount{ 1 }
         }
      };

      command_buffer.pipelineBarrier(source_stage_mask, destination_stage_mask, {}, {}, {}, { image_memory_barrier });

      end_single_time_commands(command_buffer);
   }

   void Application::record_command_buffer(vk::CommandBuffer const command_buffer, std::uint32_t const image_index) const
   {
      command_buffer.begin(vk::CommandBufferBeginInfo{});

      std::array<vk::ClearValue, 2> constexpr clear_color_values{
         {
            { vk::ClearColorValue{ std::array{ 0.0f, 0.0f, 0.0f, 1.0f } } },
            {
               vk::ClearDepthStencilValue{
                  .depth{ 1.0f }
               }
            }
         }
      };

      command_buffer.beginRenderPass({
         .renderPass{ render_pass_ },
         .framebuffer{ swap_chain_framebuffers_[image_index] },
         .renderArea{
            .extent{ swap_chain_extent_ }
         },
         .clearValueCount{ static_cast<std::uint32_t>(clear_color_values.size()) },
         .pClearValues{ clear_color_values.data() }
      }, vk::SubpassContents::eInline);

      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

      command_buffer.bindVertexBuffers(0, { vertex_buffer_.first }, { {} });
      command_buffer.bindIndexBuffer(index_buffer_.first, 0, vk::IndexType::eUint32);
      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,
         { descriptor_sets_[image_index] }, {});

      command_buffer.setViewport(0, {
         {
            .width{ static_cast<float>(swap_chain_extent_.width) },
            .height{ static_cast<float>(swap_chain_extent_.height) },
            .maxDepth{ 1.0f }
         }
      });

      command_buffer.setScissor(0, {
         {
            .extent{ swap_chain_extent_ }
         }
      });

      command_buffer.drawIndexed(static_cast<std::uint32_t>(model_.second.size()), 1, 0, 0, 0);

      command_buffer.endRenderPass();

      command_buffer.end();
   }

   void Application::draw_frame()
   {
      if (device_.waitForFences({ command_buffer_executed_fences_[current_frame_] }, true,
         std::numeric_limits<std::uint64_t>::max()) not_eq vk::Result::eSuccess)
         throw std::runtime_error("failed to wait for fences!");

      auto&& [result, image_index]{
         device_.acquireNextImageKHR(swap_chain_, std::numeric_limits<std::uint64_t>::max(),
            image_available_semaphores_[current_frame_])
      };

      device_.resetFences({ command_buffer_executed_fences_[current_frame_] });

      command_buffers_[current_frame_].reset();
      record_command_buffer(command_buffers_[current_frame_], image_index);

      update_uniform_buffer(current_frame_);

      std::array<vk::PipelineStageFlags, 1> constexpr wait_stages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
      graphics_queue_.submit({
         {
            .waitSemaphoreCount{ 1 },
            .pWaitSemaphores{ &image_available_semaphores_[current_frame_] },
            .pWaitDstStageMask{ wait_stages.data() },
            .commandBufferCount{ 1 },
            .pCommandBuffers{ &command_buffers_[current_frame_] },
            .signalSemaphoreCount{ 1 },
            .pSignalSemaphores{ &render_finished_semaphores_[current_frame_] },
         }
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

   void Application::update_uniform_buffer(std::size_t const current_frame) const
   {
      static auto start_time{ std::chrono::high_resolution_clock::now() };

      auto const current_time{ std::chrono::high_resolution_clock::now() };
      float const time{ std::chrono::duration<float>(current_time - start_time).count() };

      UniformBufferObject const uniform_buffer_object{
         .model{ glm::rotate<float, glm::defaultp>({ 1.0f }, time * glm::half_pi<float>(), { 0.0f, 1.0f, 0.0f }) },
         .view{ glm::lookAt<float, glm::defaultp>({ 0.0f, 2.0f, -2.0 }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }) },
         .projection{
            glm::perspective(glm::radians(45.0f), swap_chain_extent_.width / static_cast<float>(swap_chain_extent_.height),
               0.1f, 8.0f)
         }
      };

      VmaAllocationInfo allocation_info;
      vmaGetAllocationInfo(allocator_, uniform_buffers_[current_frame].second, &allocation_info);
      std::memcpy(allocation_info.pMappedData, &uniform_buffer_object, sizeof(uniform_buffer_object));
   }

   void Application::recreate_swapchain()
   {
      device_.waitIdle();

      for (vk::Framebuffer const framebuffer : swap_chain_framebuffers_)
         device_.destroyFramebuffer(framebuffer);

      device_.destroyImageView(depth_image_view_);
      vmaDestroyImage(allocator_, depth_image_.first, depth_image_.second);

      for (vk::ImageView const image_view : swap_chain_image_views_)
         device_.destroyImageView(image_view);

      device_.destroySwapchainKHR(swap_chain_);

      swap_chain_format_ = pick_swap_chain_format();
      swap_chain_extent_ = pick_swap_chain_extent();

      SDL_Event event;
      while (not swap_chain_extent_.width or not swap_chain_extent_.height)
      {
         SDL_WaitEvent(&event);
         swap_chain_extent_ = pick_swap_chain_extent();
      }

      swap_chain_ = create_swap_chain();
      swap_chain_images_ = device_.getSwapchainImagesKHR(swap_chain_);
      swap_chain_image_views_ = create_image_views();
      depth_image_ = create_depth_image();
      depth_image_view_ = create_depth_image_view();
      swap_chain_framebuffers_ = create_frame_buffers();
   }
}