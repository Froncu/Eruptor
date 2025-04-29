#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   class Context final
   {
      friend class ContextBuilder;

      public:
         Context(Context const&) = delete;
         Context(Context&&) = delete;

         ~Context() = default;

         Context& operator=(Context const&) = delete;
         Context& operator=(Context&&) = delete;

         [[nodiscard]] vk::raii::Context const& context() const;
         [[nodiscard]] vk::raii::Instance const& instance() const;
         [[nodiscard]] vk::raii::DebugUtilsMessengerEXT const& debug_messenger() const;

      private:
         Context(vk::raii::Context context, vk::raii::Instance instance, vk::raii::DebugUtilsMessengerEXT debug_messenger);

         vk::raii::Context context_;
         vk::raii::Instance instance_;
         vk::raii::DebugUtilsMessengerEXT debug_messenger_;
   };
}

#endif