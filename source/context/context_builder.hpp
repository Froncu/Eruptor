#ifndef CONTEXT_BUILDER_HPP
#define CONTEXT_BUILDER_HPP

#include "context.hpp"

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

         ContextBuilder& enable_validation_layer(std::string validation_layer_name);
         ContextBuilder& enable_validation_layers(std::initializer_list<std::string> validation_layer_names);
         ContextBuilder& enable_extension(std::string extenion_name);
         ContextBuilder& enable_extensions(std::initializer_list<std::string> extenion_names);

         [[nodiscard]] Context build();

      private:
         std::unordered_set<std::string> validation_layer_names_{};
         std::unordered_set<std::string> extension_names_{};
   };
}

#endif