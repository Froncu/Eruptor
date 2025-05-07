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

         [[nodiscard]] SwapChain build(Device const& device, Window const& window, std::span<DeviceQueue const> queues);

      private:
         [[nodiscard]] vk::raii::SwapchainKHR create_swap_chain(Device const& device, Window const& window,
            std::span<DeviceQueue const> queues);
         [[nodiscard]] std::vector<Image> create_images(Device const& device, vk::raii::SwapchainKHR const& swap_chain);

         vk::SurfaceFormatKHR format_{ vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
         vk::PresentModeKHR present_mode_{ vk::PresentModeKHR::eFifo };
         SwapChain const* old_swap_chain_{};
   };
}

#endif