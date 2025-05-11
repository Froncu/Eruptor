#include "swap_chain.hpp"

namespace eru
{
   SwapChain::SwapChain(vk::raii::SwapchainKHR swap_chain, std::vector<Image> images, vk::Extent2D const extent)
      : swap_chain_{ std::move(swap_chain) }
      , images_{ std::move(images) }
      , extent_{ extent }
   {
   }

   vk::raii::SwapchainKHR const& SwapChain::swap_chain() const
   {
      return swap_chain_;
   }

   std::vector<Image> const& SwapChain::images() const
   {
      return images_;
   }

   vk::Extent2D SwapChain::extent() const
   {
      return extent_;
   }
}