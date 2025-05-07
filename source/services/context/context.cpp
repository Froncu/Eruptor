#include "context.hpp"

namespace eru
{
   Context::Context(vk::raii::Context context, vk::raii::Instance instance, vk::raii::DebugUtilsMessengerEXT debug_messenger,
      std::uint32_t application_version)
      : context_{ std::move(context) }
      , instance_{ std::move(instance) }
      , debug_messenger_{ std::move(debug_messenger) }
      , api_version_{ application_version }
   {
   }

   vk::raii::Context const& Context::context() const
   {
      return context_;
   }

   vk::raii::Instance const& Context::instance() const
   {
      return instance_;
   }

   vk::raii::DebugUtilsMessengerEXT const& Context::debug_messenger() const
   {
      return debug_messenger_;
   }

   std::uint32_t Context::api_version() const
   {
      return api_version_;
   }
}