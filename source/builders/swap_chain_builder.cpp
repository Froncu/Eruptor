#include "swap_chain_builder.hpp"

namespace eru
{
   vk::Extent2D SwapChainBuilder::pick_extent(Device const& device, Window const& window)
   {
      vk::SurfaceCapabilitiesKHR const surface_capabilities{
         device.physical_device().getSurfaceCapabilitiesKHR(window.surface())
      };

      vk::Extent2D extent;
      if (surface_capabilities.currentExtent.width not_eq std::numeric_limits<std::uint32_t>::max())
         extent = surface_capabilities.currentExtent;
      else
      {
         extent = window.extent();
         extent = {
            .width{
               std::clamp(extent.width,
                  surface_capabilities.minImageExtent.width,
                  surface_capabilities.maxImageExtent.width)
            },
            .height{
               std::clamp(extent.height,
                  surface_capabilities.minImageExtent.height,
                  surface_capabilities.maxImageExtent.height)
            }
         };
      }

      return extent;
   }

   SwapChainBuilder& SwapChainBuilder::change_format(vk::SurfaceFormatKHR const format)
   {
      format_ = format;
      return *this;
   }

   SwapChainBuilder& SwapChainBuilder::change_present_mode(vk::PresentModeKHR const present_mode)
   {
      present_mode_ = present_mode;
      return *this;
   }

   SwapChainBuilder& SwapChainBuilder::change_old_swap_chain(SwapChain const* const old_swap_chain)
   {
      old_swap_chain_ = old_swap_chain;
      return *this;
   }

   SwapChain SwapChainBuilder::build(Device const& device, Window const& window,
      std::span<DeviceQueue const> const queues) const
   {
      vk::Extent2D const extent{ pick_extent(device, window) };
      vk::raii::SwapchainKHR swap_chain{ create_swap_chain(device, window, queues, extent) };
      return { std::move(swap_chain), create_images(device, swap_chain, extent), extent };
   }

   vk::raii::SwapchainKHR SwapChainBuilder::create_swap_chain(Device const& device, Window const& window,
      std::span<DeviceQueue const> queues, vk::Extent2D extent) const
   {
      vk::SurfaceCapabilitiesKHR const surface_capabilities{
         device.physical_device().getSurfaceCapabilitiesKHR(window.surface())
      };

      auto const queue_family_indices_view{
         std::views::transform(queues,
            [](DeviceQueue const& queue)
            {
               return queue.family_index();
            })
      };

      std::vector<std::uint32_t> queue_family_indices{
         queue_family_indices_view.begin(), queue_family_indices_view.end()
      };

      std::ranges::sort(queue_family_indices);
      auto const [new_end, old_end]{ std::ranges::unique(queue_family_indices) };
      queue_family_indices.erase(new_end, old_end);

      return
         device.device().createSwapchainKHR({
            .surface{ *window.surface() },
            .minImageCount{ surface_capabilities.minImageCount },
            .imageFormat{ format_.format },
            .imageColorSpace{ format_.colorSpace },
            .imageExtent{ extent },
            .imageArrayLayers{ 1 },
            .imageUsage{ vk::ImageUsageFlagBits::eColorAttachment },
            .imageSharingMode{ queue_family_indices.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive },
            .queueFamilyIndexCount{ static_cast<std::uint32_t>(queue_family_indices.size()) },
            .pQueueFamilyIndices{ queue_family_indices.data() },
            .preTransform{ surface_capabilities.currentTransform },
            .compositeAlpha{ vk::CompositeAlphaFlagBitsKHR::eOpaque },
            .presentMode{ present_mode_ },
            .clipped{ true },
            .oldSwapchain{ old_swap_chain_ ? *old_swap_chain_->swap_chain() : nullptr }
         });
   }

   std::vector<Image> SwapChainBuilder::create_images(Device const& device, vk::raii::SwapchainKHR const& swap_chain,
      vk::Extent2D const extent) const
   {
      std::vector<Image> images{};
      for (vk::Image const image : swap_chain.getImages())
         images.push_back({
            image,
            device.device().createImageView({
               .image{ image },
               .viewType{ vk::ImageViewType::e2D },
               .format{ format_.format },
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
            }),
            format_.format,
            vk::ImageLayout::eUndefined,
            vk::Extent3D{
               .width{ extent.width },
               .height{ extent.height },
               .depth{ 1 }
            }
         });

      return images;
   };
}