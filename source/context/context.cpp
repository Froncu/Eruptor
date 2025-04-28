#include "context.hpp"

namespace eru
{
   Context::Context(vk::raii::Context context, vk::raii::Instance instance, vk::raii::DebugUtilsMessengerEXT debug_messenger)
      : context_{ std::move(context) }
      , instance_{ std::move(instance) }
      , debug_messenger_{ std::move(debug_messenger) }
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
}