#ifndef RENDERER_CONTEXT_HPP
#define RENDERER_CONTEXT_HPP

#include "eruptor/pch.hpp"
#include "eruptor/runtime_assert.hpp"

namespace eru
{
   class RendererContext final
   {
      public:
         ERU_API RendererContext(std::initializer_list<char const* const> instance_extension_names = {}, bool create_messenger = DEBUG_BUILD);
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