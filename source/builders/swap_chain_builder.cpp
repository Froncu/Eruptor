#include "swap_chain_builder.hpp"

namespace eru
{
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
      std::span<DeviceQueue const> queues)
   {
      vk::raii::SwapchainKHR swap_chain{ create_swap_chain(device, window, queues) };
      return { std::move(swap_chain), create_images(device, swap_chain) };
   }

   vk::raii::SwapchainKHR SwapChainBuilder::create_swap_chain(Device const& device, Window const& window,
      std::span<DeviceQueue const> queues)
   {
      vk::SurfaceCapabilitiesKHR const surface_capabilities{
         device.physical_device().getSurfaceCapabilitiesKHR(window.surface())
      };

      vk::Extent2D swap_chain_extent;
      if (surface_capabilities.currentExtent.width not_eq std::numeric_limits<std::uint32_t>::max())
         swap_chain_extent = surface_capabilities.currentExtent;
      else
      {
         swap_chain_extent = window.extent();
         swap_chain_extent = {
            .width{
               std::clamp(swap_chain_extent.width,
                  surface_capabilities.minImageExtent.width,
                  surface_capabilities.maxImageExtent.width)
            },
            .height{
               std::clamp(swap_chain_extent.height,
                  surface_capabilities.minImageExtent.height,
                  surface_capabilities.maxImageExtent.height)
            }
         };
      }

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
            .imageExtent{ swap_chain_extent },
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

   std::vector<Image> SwapChainBuilder::create_images(Device const& device, vk::raii::SwapchainKHR const& swap_chain)
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
            vk::ImageLayout::eUndefined
         });

      return images;
   };
}