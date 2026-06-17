#include "eruptor/pass_key.hpp"
#include "eruptor/runtime_assert.hpp"
#include "eruptor/swap_chain.hpp"
#include "eruptor/context.hpp"

namespace eru
{
   SwapChain::SwapChain(PassKey<Window> const, vk::raii::SurfaceKHR const& surface, Description const& parameters)
      : surface_{ surface }
      , image_available_semaphores_{ { context_.create_semaphores(parameters.frames_in_flight) } }
      , swap_chain_{ swap_chain(parameters) }
      , swap_chain_image_views_{ swap_chain_image_views(parameters.format) }
   {
   }

   SwapChain::~SwapChain()
   {
      vk::Result const result{ context_.device.waitIdle() };
      RUNTIME_ASSERT(result == vk::Result::eSuccess,
         std::format("failed to wait idle on the device! ({})", to_string(result)));
   }

   auto SwapChain::swap_chain(Description const& parameters) const -> vk::raii::SwapchainKHR
   {
      // TODO: use `vk::StructureChain` and `getSurfaceCapabilities2KHR` for more functionality
      vk::ResultValue surface_capabilities{ context_.physical_device.getSurfaceCapabilitiesKHR(surface_) };
      RUNTIME_ASSERT(surface_capabilities.has_value(),
         std::format("failed to query surface capabilities! ({})", to_string(surface_capabilities.result)));

      std::uint32_t minimal_image_count{ std::max(parameters.minimal_image_count, surface_capabilities->minImageCount) };
      if (surface_capabilities->maxImageCount)
         minimal_image_count = std::min(minimal_image_count, surface_capabilities->maxImageCount);

      // TODO: use `vk::StructureChain` and `getSurfaceFormats2KHR` for more functionality
      vk::ResultValue const available_surface_formats{ context_.physical_device.getSurfaceFormatsKHR(surface_) };
      RUNTIME_ASSERT(available_surface_formats.has_value(),
         std::format("failed to query available surface formats! ({})", to_string(available_surface_formats.result)));

      RUNTIME_ASSERT(not std::ranges::empty(*available_surface_formats),
         "no surface formats are available!");

      auto surface_format{
         std::ranges::find_if(
            *available_surface_formats,
            [&parameters](vk::SurfaceFormatKHR const& available_surface_format)
            {
               auto const [format, color_space]{ available_surface_format };
               return format == parameters.format
                  and color_space == parameters.color_space;
            })
      };
      if (surface_format == std::ranges::end(*available_surface_formats))
         surface_format = std::ranges::begin(*available_surface_formats);

      if (surface_capabilities->currentExtent.width == std::numeric_limits<std::uint32_t>::max() and surface_capabilities->currentExtent.height == std::numeric_limits<std::uint32_t>::max())
         surface_capabilities->currentExtent = {
            std::clamp<std::uint32_t>(parameters.extent.width, surface_capabilities->minImageExtent.width, surface_capabilities->maxImageExtent.width),
            std::clamp<std::uint32_t>(parameters.extent.height, surface_capabilities->minImageExtent.height, surface_capabilities->maxImageExtent.height)
         };

      // TODO: use `vk::StructureChain` for more functionality
      vk::ResultValue const available_surface_present_modes{ context_.physical_device.getSurfacePresentModesKHR(surface_) };
      RUNTIME_ASSERT(available_surface_present_modes.has_value(),
         std::format("failed to query available surface present modes! ({})",
            to_string(available_surface_present_modes.result)));

      RUNTIME_ASSERT(not std::ranges::empty(*available_surface_present_modes),
         "no present modes are available!");

      auto surface_present_mode{
         std::ranges::find_if(
            *available_surface_present_modes,
            [&parameters](vk::PresentModeKHR const& available_surface_present_mode)
            {
               return available_surface_present_mode == parameters.present_mode;
            })
      };
      if (surface_present_mode == std::ranges::end(*available_surface_present_modes))
         surface_present_mode = std::ranges::begin(*available_surface_present_modes);

      // TODO: make use of `oldSwapchain`
      vk::ResultValue swap_chain{
         context_.device.createSwapchainKHR({
            .surface{ *surface_ },
            .minImageCount{ minimal_image_count },
            .imageFormat{ surface_format->format },
            .imageColorSpace{ surface_format->colorSpace },
            .imageExtent{ surface_capabilities->currentExtent },
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
      RUNTIME_ASSERT(swap_chain.has_value(),
         std::format("failed to create a swap chain! ({})", to_string(swap_chain.result)));

      return std::move(*swap_chain);
   }

   auto SwapChain::swap_chain_images() const -> std::vector<vk::Image>
   {
      vk::ResultValue swap_chain_images{ swap_chain_.getImages() };
      RUNTIME_ASSERT(swap_chain_images.has_value(),
         std::format("failed to retrieve swap chain images! ({})", to_string(swap_chain_images.result)));

      return std::move(*swap_chain_images);
   }

   auto SwapChain::swap_chain_image_views(vk::Format const format) const -> std::vector<vk::raii::ImageView>
   {
      vk::ImageViewCreateInfo create_info{
         .viewType{ vk::ImageViewType::e2D },
         .format{ format },
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
         vk::ResultValue image_view{ context_.device.createImageView(create_info) };
         RUNTIME_ASSERT(image_view.has_value(),
            std::format("failed to create a swap chain image view! ({})", to_string(image_view.result)));

         image_views.push_back(std::move(*image_view));
      }

      return image_views;
   }
}