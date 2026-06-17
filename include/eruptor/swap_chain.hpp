#ifndef SWAP_CHAIN_HPP
#define SWAP_CHAIN_HPP

#include "eruptor/context.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Context;
   class Window;

   class SwapChain final
   {
      public:
         struct Description final
         {
            std::uint32_t frames_in_flight{ 1u };
            std::uint32_t minimal_image_count{ 3u };
            vk::Extent2D extent{};
            vk::Format format{ vk::Format::eB8G8R8A8Srgb };
            vk::ColorSpaceKHR color_space{ vk::ColorSpaceKHR::eSrgbNonlinear };
            vk::PresentModeKHR present_mode{ vk::PresentModeKHR::eMailbox };
         };

         SwapChain(PassKey<Window>, vk::raii::SurfaceKHR const& surface, Description const& parameters = {});
         SwapChain(SwapChain const&) = delete;
         SwapChain(SwapChain&&) = default;

         ~SwapChain();

         auto operator=(SwapChain const&) -> SwapChain& = delete;
         auto operator=(SwapChain&&) -> SwapChain& = delete;

      private:
         [[nodiscard]] auto swap_chain(Description const& parameters) const -> vk::raii::SwapchainKHR;
         [[nodiscard]] auto swap_chain_images() const -> std::vector<vk::Image>;
         [[nodiscard]] auto swap_chain_image_views(vk::Format format) const -> std::vector<vk::raii::ImageView>;

         Context const& context_{ Locator::get<Context>() };

         vk::raii::SurfaceKHR const& surface_;
         std::vector<vk::raii::Semaphore> const image_available_semaphores_;

         vk::raii::SwapchainKHR swap_chain_;
         std::vector<vk::Image> swap_chain_images_{ swap_chain_images() };
         std::vector<vk::raii::ImageView> swap_chain_image_views_;
   };
}

#endif