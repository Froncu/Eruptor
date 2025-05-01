#include "swap_chain.hpp"

namespace eru
{
   SwapChain::SwapChain(vk::raii::SwapchainKHR swap_chain, std::vector<Image> images)
      : swap_chain_{ std::move(swap_chain) }
      , images_{ std::move(images) }
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
}