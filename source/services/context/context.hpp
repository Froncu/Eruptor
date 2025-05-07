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
         Context(Context&&) = default;

         ~Context() = default;

         Context& operator=(Context const&) = delete;
         Context& operator=(Context&&) = default;

         [[nodiscard]] vk::raii::Context const& context() const;
         [[nodiscard]] vk::raii::Instance const& instance() const;
         [[nodiscard]] vk::raii::DebugUtilsMessengerEXT const& debug_messenger() const;
         [[nodiscard]] std::uint32_t api_version() const;

      private:
         Context(vk::raii::Context context, vk::raii::Instance instance, vk::raii::DebugUtilsMessengerEXT debug_messenger,
            std::uint32_t application_version);

         vk::raii::Context context_;
         vk::raii::Instance instance_;
         vk::raii::DebugUtilsMessengerEXT debug_messenger_;

         std::uint32_t api_version_;
   };
}

#endif