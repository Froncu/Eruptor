#include "image_view_builder.hpp"
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

   std::vector<ImageView> SwapChainBuilder::create_image_views(Device const& device, std::vector<Image> const& images)
   {
      ImageViewBuilder image_view_builder{};
      std::vector<ImageView> views{};
      views.reserve(images.size());

      for (Image const& image : images)
      {
         image_view_builder.change_format(image.info().format);
         views.push_back(image_view_builder.build(device, image));
      }

      return views;
   };

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
      std::vector images{ create_images(swap_chain, extent, queues) };
      return { std::move(swap_chain), std::move(images), create_image_views(device, images), extent };
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

   std::vector<Image> SwapChainBuilder::create_images(vk::raii::SwapchainKHR const& swap_chain, vk::Extent2D const extent,
      std::span<DeviceQueue const> queues) const
   {
      std::vector<Image> images{};
      std::vector<std::uint32_t> queue_family_indices{};

      for (DeviceQueue const& queue : queues)
         queue_family_indices.push_back(queue.family_index());

      std::ranges::sort(queue_family_indices);
      queue_family_indices.erase(std::ranges::unique(queue_family_indices).begin(), queue_family_indices.end());

      for (vk::Image const image : swap_chain.getImages())
         images.push_back({
            image,
            vk::ImageCreateInfo{
               .imageType{ vk::ImageType::e2D },
               .format{ format_.format },
               .extent{
                  .width{ extent.width },
                  .height{ extent.height },
                  .depth{ 1 }
               },
               .mipLevels{ 1 },
               .arrayLayers{ 1 },
               .samples{ vk::SampleCountFlagBits::e1 },
               .tiling{ vk::ImageTiling::eOptimal },
               .usage{ vk::ImageUsageFlagBits::eColorAttachment },
               .sharingMode{ vk::SharingMode::eExclusive },
               .queueFamilyIndexCount{ static_cast<std::uint32_t>(queue_family_indices.size()) },
               .pQueueFamilyIndices{ queue_family_indices.data() },
               .initialLayout{ vk::ImageLayout::eUndefined }
            }
         });

      return images;
   }
}