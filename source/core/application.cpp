#include "eruptor/application.hpp"
#include "eruptor/exception.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"
#include "eruptor/runtime_assert.hpp"

#include "core/dependencies.hpp"

namespace eru
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

   auto Application::tick() -> bool
   {
      vk::raii::CommandBuffer const& command_buffer{ command_buffers_[frame_index_] };
      vk::raii::Semaphore const& image_available_semaphore{ image_available_semaphores_[frame_index_] };
      vk::raii::Semaphore const& command_buffer_finished_semaphore{ command_buffer_finished_semaphores_[frame_index_] };
      vk::raii::Fence const& presentation_finished_fence{ presentation_finished_fences_[frame_index_] };
      auto& [model, view, projection]{ *uniform_buffer_mapped_[frame_index_] };

      static auto start_time{ std::chrono::high_resolution_clock::now() };

      auto const current_time{ std::chrono::high_resolution_clock::now() };
      float const time{ std::chrono::duration<float>(current_time - start_time).count() };

      model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      projection = glm::perspective(glm::radians(45.0f),
         static_cast<float>(surface_extent_.width) / static_cast<float>(surface_extent_.height), 0.1f, 10.0f);
      projection[1][1] *= -1;

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
      command_buffer.bindIndexBuffer(*index_buffer_, 0, vk::IndexType::eUint16);
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

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0, *descriptor_sets_[frame_index_],
         nullptr);
      command_buffer.drawIndexed(static_cast<std::uint32_t>(indices_.size()), 1, 0, 0, 0);
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
            return keep_ticking;

         default:
            runtime_assert(result == vk::Result::eSuccess,
               std::format("failed to submit command buffer! ({})", to_string(result)));
            break;
      }

      ++frame_index_ %= FRAMES_IN_FLIGHT;
      return keep_ticking;
   }

   auto Application::poll() -> void
   {
      glfwPollEvents();
   }

   Application::Application(std::string_view const name, std::uint32_t const version)
      : instance_{ instance(name, version) }
   {
      vk::Result result{ vertex_buffer_.bindMemory(vertex_buffer_memory_, 0) };
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to bind vertex buffer's memory! ({})", to_string(result)));

      vk::DeviceSize const vertex_buffer_size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() };
      vk::raii::Buffer const vertex_staging_buffer{
         buffer({
            .size{ vertex_buffer_size },
            .usage{ vk::BufferUsageFlagBits::eTransferSrc },
            .sharingMode{ vk::SharingMode::eExclusive }
         })
      };

      vk::raii::DeviceMemory const vertex_staging_buffer_memory{
         memory(vertex_staging_buffer.getMemoryRequirements(),
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      result = vertex_staging_buffer.bindMemory(vertex_staging_buffer_memory, 0);
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to bind vertex staging buffer's memory! ({})", to_string(result)));

      vk::ResultValue mapped_memory{ vertex_staging_buffer_memory.mapMemory(0, vertex_buffer_size) };
      runtime_assert(mapped_memory.has_value(),
         std::format("failed to map vertex staging buffer's memory! ({})", to_string(mapped_memory.result)));

      std::memcpy(*mapped_memory, vertices_.data(), vertex_buffer_size);
      vertex_staging_buffer_memory.unmapMemory();

      //

      result = index_buffer_.bindMemory(index_buffer_memory_, 0);
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to bind index buffer's memory! ({})", to_string(result)));

      vk::DeviceSize const index_buffer_size{ sizeof(decltype(indices_)::value_type) * indices_.size() };
      vk::raii::Buffer const index_staging_buffer{
         buffer({
            .size{ index_buffer_size },
            .usage{ vk::BufferUsageFlagBits::eTransferSrc },
            .sharingMode{ vk::SharingMode::eExclusive }
         })
      };

      vk::raii::DeviceMemory const index_staging_buffer_memory{
         memory(index_staging_buffer.getMemoryRequirements(),
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
      };

      result = index_staging_buffer.bindMemory(index_staging_buffer_memory, 0);
      runtime_assert(result == vk::Result::eSuccess,
         std::format("failed to bind index staging buffer's memory! ({})", to_string(result)));

      mapped_memory = index_staging_buffer_memory.mapMemory(0, index_buffer_size);
      runtime_assert(mapped_memory.has_value(),
         std::format("failed to map index staging buffer's memory! ({})", to_string(mapped_memory.result)));

      std::memcpy(*mapped_memory, indices_.data(), index_buffer_size);
      index_staging_buffer_memory.unmapMemory();

      //

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

      std::array const vertex_buffer_copy_regions{
         std::to_array<vk::BufferCopy2>({
            {
               .size{ vertex_buffer_size }
            }
         })
      };
      command_buffers->front().copyBuffer2({
         .srcBuffer{ vertex_staging_buffer },
         .dstBuffer{ vertex_buffer_ },
         .regionCount{ static_cast<std::uint32_t>(std::ranges::size(vertex_buffer_copy_regions)) },
         .pRegions{ std::ranges::data(vertex_buffer_copy_regions) }
      });

      std::array const index_buffer_copy_regions{
         std::to_array<vk::BufferCopy2>({
            {
               .size{ index_buffer_size }
            }
         })
      };
      command_buffers->front().copyBuffer2({
         .srcBuffer{ index_staging_buffer },
         .dstBuffer{ index_buffer_ },
         .regionCount{ static_cast<std::uint32_t>(std::ranges::size(index_buffer_copy_regions)) },
         .pRegions{ std::ranges::data(index_buffer_copy_regions) }
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

      //

      uniform_buffer_mapped_.reserve(FRAMES_IN_FLIGHT);
      std::vector<vk::DescriptorBufferInfo> buffer_infos{};
      buffer_infos.reserve(FRAMES_IN_FLIGHT);
      std::vector<vk::WriteDescriptorSet> writes{};
      writes.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
      {
         uniform_buffers_[index].bindMemory(uniform_buffer_memories_[index], 0);

         mapped_memory = uniform_buffer_memories_[index].mapMemory(0, sizeof(UniformBufferObject));
         runtime_assert(mapped_memory.has_value(),
            std::format("failed to map a uniform buffer's memory! ({})", to_string(mapped_memory.result)));

         uniform_buffer_mapped_.push_back(static_cast<UniformBufferObject*>(*mapped_memory));

         buffer_infos.push_back({
            .buffer{ uniform_buffers_[index] },
            .offset{},
            .range{ vk::WholeSize }
         });

         writes.push_back({
            .dstSet{ descriptor_sets_[index] },
            .dstBinding{},
            .dstArrayElement{},
            .descriptorCount{ 1 },
            .descriptorType{ vk::DescriptorType::eUniformBuffer },
            .pImageInfo{},
            .pBufferInfo{ &buffer_infos[index] },
            .pTexelBufferView{}
         });
      }

      device_.updateDescriptorSets(writes, {});
   }

   auto Application::instance(std::string_view const name, std::uint32_t const version) const -> vk::raii::Instance
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

      vk::ResultValue instance{
         vulkan_context_.createInstance({
            .flags{},
            .pApplicationInfo{ &app_info },
            .enabledLayerCount{},
            .ppEnabledLayerNames{},
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(extension_names) }
         })
      };
      runtime_assert(instance.has_value(),
         std::format("failed to create a Vulkan instance! ({})", to_string(instance.result)));

      return std::move(*instance);
   }

   auto Application::debug_messenger() const -> vk::raii::DebugUtilsMessengerEXT
   {
      vk::ResultValue debug_messenger{
         instance_.createDebugUtilsMessengerEXT({
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
      runtime_assert(debug_messenger.has_value(),
         std::format("failed to create a debug messenger! ({})", to_string(debug_messenger.result)));

      return std::move(*debug_messenger);
   }

   auto Application::surface() const -> vk::raii::SurfaceKHR
   {
      VkSurfaceKHR surface;
      glfwCreateWindowSurface(*instance_, &window_.native(), nullptr, &surface);
      return { instance_, surface };
   }

   auto Application::physical_device() const -> vk::raii::PhysicalDevice
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

   auto Application::queue_family_index() const -> std::uint32_t
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

   auto Application::device() const -> vk::raii::Device
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
            .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(device_extension_names)) },
            .ppEnabledExtensionNames{ std::ranges::data(device_extension_names) },
         })
      };
      runtime_assert(device.has_value(),
         std::format("failed to create a device! ({})", to_string(device.result)));

      return std::move(*device);
   }

   auto Application::queue() const -> vk::raii::Queue
   {
      return device_.getQueue2({
         .queueFamilyIndex{ queue_family_index_ },
         .queueIndex{ 0 }
      });
   }

   auto Application::surface_format() const -> vk::SurfaceFormatKHR
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

   auto Application::surface_extent() const -> vk::Extent2D
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

   auto Application::swap_chain() const -> vk::raii::SwapchainKHR
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

   auto Application::swap_chain_images() const -> std::vector<vk::Image>
   {
      vk::ResultValue swap_chain_images{ swap_chain_.getImages() };
      runtime_assert(swap_chain_images.has_value(),
         std::format("failed to retrieve swap chain images! ({})", to_string(swap_chain_images.result)));

      return std::move(*swap_chain_images);
   }

   auto Application::swap_chain_image_views() const -> std::vector<vk::raii::ImageView>
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

   auto Application::descriptor_set_layout() const -> vk::raii::DescriptorSetLayout
   {
      std::array constexpr bindings{
         std::to_array<vk::DescriptorSetLayoutBinding>({
            {
               .binding{ 0 },
               .descriptorType{ vk::DescriptorType::eUniformBuffer },
               .descriptorCount{ 1 },
               .stageFlags{ vk::ShaderStageFlagBits::eVertex }
            }
         })
      };

      vk::ResultValue descriptor_set_layout{
         device_.createDescriptorSetLayout({
            .bindingCount{ static_cast<std::uint32_t>(std::ranges::size(bindings)) },
            .pBindings{ std::ranges::data(bindings) }
         })
      };
      runtime_assert(descriptor_set_layout.has_value(),
         std::format("failed to create descriptor set layout! ({})", to_string(descriptor_set_layout.result)));

      return std::move(*descriptor_set_layout);
   }

   auto Application::pipeline_layout() const -> vk::raii::PipelineLayout
   {
      // TODO: how to pass a container of vk::raii namespaced descriptor set layouts without creating a proxy container?
      vk::ResultValue pipeline_layout{
         device_.createPipelineLayout({
            .setLayoutCount{ 1 },
            .pSetLayouts{ &*descriptor_set_layout_ }
         })
      };
      runtime_assert(pipeline_layout.has_value(),
         std::format("failed to create a pipeline layout! ({})", to_string(pipeline_layout.result)));

      return std::move(pipeline_layout.value);
   }

   auto Application::pipeline() const -> vk::raii::Pipeline
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
         .pCode{ static_cast<std::uint32_t const*>(spirv->getBufferPointer()) }
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
         .topology{ vk::PrimitiveTopology::eTriangleStrip }
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
         .frontFace{ vk::FrontFace::eCounterClockwise },
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

   auto Application::command_pool() const -> vk::raii::CommandPool
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

   auto Application::command_buffers() const -> std::vector<vk::raii::CommandBuffer>
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

   auto Application::semaphores() const -> std::vector<vk::raii::Semaphore>
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

   auto Application::fences() const -> std::vector<vk::raii::Fence>
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

   auto Application::buffer(vk::BufferCreateInfo const& create_info) const -> vk::raii::Buffer
   {
      vk::ResultValue buffer{ device_.createBuffer(create_info) };
      runtime_assert(buffer.has_value(),
         std::format("failed to create vertex buffer! ({})", to_string(buffer.result)));

      return std::move(*buffer);
   }

   auto Application::memory(vk::MemoryRequirements const& requirements,
      vk::MemoryPropertyFlags const properties) const -> vk::raii::DeviceMemory
   {
      vk::PhysicalDeviceMemoryProperties2 const available_properties{ physical_device_.getMemoryProperties2() };
      std::bitset<sizeof(requirements.memoryTypeBits) * 8> const type_bits{ requirements.memoryTypeBits };

      std::uint32_t index{};
      for (; index < available_properties.memoryProperties.memoryTypeCount; ++index)
         if (type_bits[index]
            and (available_properties.memoryProperties.memoryTypes[index].propertyFlags & properties) == properties)
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

   auto Application::vertex_buffer() const -> vk::raii::Buffer
   {
      return buffer({
         .size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() },
         .usage{ vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst },
         .sharingMode{ vk::SharingMode::eExclusive }
      });
   }

   auto Application::vertex_buffer_memory() const -> vk::raii::DeviceMemory
   {
      return memory(vertex_buffer_.getMemoryRequirements(), vk::MemoryPropertyFlagBits::eDeviceLocal);
   }

   auto Application::index_buffer() const -> vk::raii::Buffer
   {
      return buffer({
         .size{ sizeof(decltype(vertices_)::value_type) * vertices_.size() },
         .usage{ vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst },
         .sharingMode{ vk::SharingMode::eExclusive }
      });
   }

   auto Application::index_buffer_memory() const -> vk::raii::DeviceMemory
   {
      return memory(index_buffer_.getMemoryRequirements(), vk::MemoryPropertyFlagBits::eDeviceLocal);
   }

   auto Application::uniform_buffers() const -> std::vector<vk::raii::Buffer>
   {
      std::vector<vk::raii::Buffer> uniform_buffers{};
      uniform_buffers.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
         uniform_buffers.push_back(
            buffer({
               .size{ sizeof(UniformBufferObject) },
               .usage{ vk::BufferUsageFlagBits::eUniformBuffer }
            }));

      return uniform_buffers;
   }

   auto Application::uniform_buffer_memories() const -> std::vector<vk::raii::DeviceMemory>
   {
      std::vector<vk::raii::DeviceMemory> uniform_buffer_memories{};
      uniform_buffer_memories.reserve(FRAMES_IN_FLIGHT);
      for (std::size_t index{}; index < FRAMES_IN_FLIGHT; ++index)
         uniform_buffer_memories.push_back(
            memory(uniform_buffers_[index].getMemoryRequirements(),
               vk::MemoryPropertyFlagBits::eDeviceLocal |
               vk::MemoryPropertyFlagBits::eHostVisible |
               vk::MemoryPropertyFlagBits::eHostCoherent));

      return uniform_buffer_memories;
   }

   auto Application::texture() const -> vk::raii::Image
   {
      // stbi_load
      //
      // ktxTexture2* texture;
      // ktxTextureCreateInfo const create_info{
      //    .vkFormat{ VK_FORMAT_R8G8B8A8_SRGB },
      //    .baseWidth{ 512 },
      //    .baseHeight{ 512 },
      //    .baseDepth{ 1 },
      //    .numDimensions{ 2 },
      //    .numLevels{ 1 },
      //    .numLayers{ 1 },
      //    .numFaces{ 1 },
      //    .isArray{ KTX_FALSE },
      //    .generateMipmaps{ KTX_FALSE }
      // };
      //
      // ktxTexture2_CreateFromNamedFile("assets/textures/text.ktx2", &create_info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
      //
      // ktx_error_code_e result{ ktxTexture2_Create(&create_info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture) };
      // runtime_assert(result == KTX_SUCCESS,
      //    std::format("failed to load create ktx texture! ({})", ktxErrorString(result)));
      //
      // std::vector<std::uint8_t> image_bytes{};
      // if (std::ifstream image{ "assets/textures/text.png", std::ios::binary }; image.is_open())
      //    image_bytes = { std::istreambuf_iterator{ image }, {} };
      //
      // ktxTexture_SetImageFromMemory(reinterpret_cast<ktxTexture*>(texture), 0, 0, 0, image_bytes.data(), image_bytes.size());
      //
      // // Get texture dimensions and data
      // uint32_t texWidth = texture->baseWidth;
      // uint32_t texHeight = texture->baseHeight;
      // ktx_size_t imageSize = texture->dataSize;
      // ktx_uint8_t* ktxTextureData = texture->pData;
      // std::ignore = texWidth;
      // std::ignore = texHeight;
      // std::ignore = imageSize;
      // std::ignore = ktxTextureData;

      return { device_, {} };
   }

   auto Application::texture_memory() const -> vk::raii::DeviceMemory
   {
      return { device_, {} };
   }

   auto Application::descriptor_pool() const -> vk::raii::DescriptorPool
   {
      std::array constexpr desciptor_pool_sizes{
         std::to_array<vk::DescriptorPoolSize>({
            {
               .type{ vk::DescriptorType::eUniformBuffer },
               .descriptorCount{ FRAMES_IN_FLIGHT }
            }
         })
      };

      vk::ResultValue descriptor_pool{
         device_.createDescriptorPool({
            .flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet },
            .maxSets{ FRAMES_IN_FLIGHT },
            .poolSizeCount{ static_cast<std::uint32_t>(std::ranges::size(desciptor_pool_sizes)) },
            .pPoolSizes{ std::ranges::data(desciptor_pool_sizes) }
         })
      };
      runtime_assert(descriptor_pool.has_value(),
         std::format("failed to create a descriptor pool! ({})", to_string(descriptor_pool.result)));

      return std::move(*descriptor_pool);
   }

   auto Application::descriptor_sets() const -> std::vector<vk::raii::DescriptorSet>
   {
      std::array<vk::DescriptorSetLayout, FRAMES_IN_FLIGHT> layouts{};
      std::ranges::fill(layouts, *descriptor_set_layout_);

      vk::ResultValue descriptor_sets{
         device_.allocateDescriptorSets({
            .descriptorPool{ descriptor_pool_ },
            .descriptorSetCount{ static_cast<std::uint32_t>(std::ranges::size(layouts)) },
            .pSetLayouts{ std::ranges::data(layouts) }
         })
      };
      runtime_assert(descriptor_sets.has_value(),
         std::format("failed to allocate descriptor sets! ({})", to_string(descriptor_sets.result)));

      return std::move(*descriptor_sets);
   }

   auto Application::recreate_swap_chain() -> void
   {
      surface_extent_ = surface_extent();
      swap_chain_.clear();
      swap_chain_ = swap_chain();
      swap_chain_images_ = swap_chain_images();
      swap_chain_image_views_ = swap_chain_image_views();
   }
}