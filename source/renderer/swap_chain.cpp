#include "swap_chain.hpp"

namespace eru
{
   SwapChain::SwapChain(vk::raii::SwapchainKHR swap_chain, std::vector<Image> images, std::vector<ImageView> image_view,
      vk::Extent2D const extent)
      : swap_chain_{ std::move(swap_chain) }
      , images_{ std::move(images) }
      , image_views_{ std::move(image_view) }
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

   std::vector<ImageView> const& SwapChain::image_views() const
   {
      return image_views_;
   }

   vk::Extent2D SwapChain::extent() const
   {
      return extent_;
   }
}