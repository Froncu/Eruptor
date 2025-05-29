#ifndef SWAP_CHAIN_HPP
#define SWAP_CHAIN_HPP

#include "erupch/erupch.hpp"
#include "image_view.hpp"
#include "renderer/image.hpp"

namespace eru
{
   class SwapChain final
   {
      friend class SwapChainBuilder;

      public:
         SwapChain(SwapChain const&) = delete;
         SwapChain(SwapChain&&) = default;

         ~SwapChain() = default;

         SwapChain& operator=(SwapChain const&) = delete;
         SwapChain& operator=(SwapChain&&) = default;

         [[nodiscard]] vk::raii::SwapchainKHR const& swap_chain() const;
         [[nodiscard]] std::vector<Image> const& images() const;
         [[nodiscard]] std::vector<ImageView> const& image_views() const;
         [[nodiscard]] vk::Extent2D extent() const;

      private:
         SwapChain(vk::raii::SwapchainKHR swap_chain, std::vector<Image> images, std::vector<ImageView> image_view,
            vk::Extent2D extent);

         vk::raii::SwapchainKHR swap_chain_;
         std::vector<Image> images_;
         std::vector<ImageView> image_views_;
         vk::Extent2D extent_;
   };
}

#endif