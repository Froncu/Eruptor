#ifndef RENDERER_CONTEXT_HPP
#define RENDERER_CONTEXT_HPP

#include "eruptor/pch.hpp"

namespace eru
{
   class RendererContext final
   {
      public:
         // NOTE: `std::initializer_list` doesn't work well with `std::string_view` as it has a constructor that takes a begin and end pointer, so we use `char const*` instead.
         ERU_API RendererContext(std::initializer_list<char const* const> requested_instance_extension_names = {},
            bool create_messenger = DEBUG_BUILD);
         RendererContext(RendererContext const&) = delete;
         RendererContext(RendererContext&&) = delete;

         ~RendererContext() = default;

         auto operator=(RendererContext const&) -> RendererContext& = delete;
         auto operator=(RendererContext&&) -> RendererContext& = delete;

         [[nodiscard]] auto instance() const -> vk::raii::Instance const&;

      private:
         vk::raii::Context const context_{};
         vk::raii::Instance const instance_;
         std::optional<vk::raii::DebugUtilsMessengerEXT> const debug_messenger_;
   };
}

#endif