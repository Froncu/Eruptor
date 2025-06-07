#ifndef CONTEXT_BUILDER_HPP
#define CONTEXT_BUILDER_HPP

#include "services/context/context.hpp"

namespace eru
{
   class ContextBuilder final
   {
      public:
         ContextBuilder() = default;
         ContextBuilder(ContextBuilder const&) = delete;
         ContextBuilder(ContextBuilder&&) = delete;

         ~ContextBuilder() = default;

         ContextBuilder& operator=(ContextBuilder const&) = delete;
         ContextBuilder& operator=(ContextBuilder&&) = delete;

         ContextBuilder& change_api_version(std::uint32_t api_version);
         ContextBuilder& enable_validation_layer(std::string_view validation_layer_name);
         ContextBuilder& enable_validation_layers(std::initializer_list<std::string_view> validation_layer_names);
         ContextBuilder& enable_instance_extension(std::string_view extenion_name);
         ContextBuilder& enable_instance_extensions(std::initializer_list<std::string_view> extenion_names);

         [[nodiscard]] Context build();

      private:
         std::uint32_t api_version_{ vk::ApiVersion };
         std::unordered_set<std::string> validation_layer_names_{};
         std::unordered_set<std::string> instance_extension_names_{};
   };
}

#endif