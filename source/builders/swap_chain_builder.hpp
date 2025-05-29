#ifndef SWAP_CHAIN_BUILDER_HPP
#define SWAP_CHAIN_BUILDER_HPP

#include "renderer/device.hpp"
#include "renderer/swap_chain.hpp"
#include "window/window.hpp"

namespace eru
{
   class SwapChainBuilder final
   {
      public:
         SwapChainBuilder() = default;
         SwapChainBuilder(SwapChainBuilder const&) = delete;
         SwapChainBuilder(SwapChainBuilder&&) = delete;

         ~SwapChainBuilder() = default;

         SwapChainBuilder& operator=(SwapChainBuilder const&) = delete;
         SwapChainBuilder& operator=(SwapChainBuilder&&) = delete;

         SwapChainBuilder& change_format(vk::SurfaceFormatKHR format);
         SwapChainBuilder& change_present_mode(vk::PresentModeKHR present_mode);
         SwapChainBuilder& change_old_swap_chain(SwapChain const* old_swap_chain);

         [[nodiscard]] SwapChain build(Device const& device, Window const& window, std::span<DeviceQueue const> queues) const;

      private:
         [[nodiscard]] static vk::Extent2D pick_extent(Device const& device, Window const& window);
         [[nodiscard]] static std::vector<ImageView> create_image_views(Device const& device, std::vector<Image> const& images);

         [[nodiscard]] vk::raii::SwapchainKHR create_swap_chain(Device const& device, Window const& window,
            std::span<DeviceQueue const> queues, vk::Extent2D extent) const;
         [[nodiscard]] std::vector<Image> create_images(vk::raii::SwapchainKHR const& swap_chain,
         vk::Extent2D extent, std::span<DeviceQueue const> queues) const;

         vk::SurfaceFormatKHR format_{ vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
         vk::PresentModeKHR present_mode_{ vk::PresentModeKHR::eFifo };
         SwapChain const* old_swap_chain_{};
   };
}

#endif