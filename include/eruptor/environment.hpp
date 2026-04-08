#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "eruptor/api.hpp"
#include "eruptor/locator.hpp"

namespace eru
{
   class Environment final
   {
      public:
         ERU_API explicit Environment(Locator::ConstructionKey);
         Environment(Environment const&) = delete;
         Environment(Environment&&) = delete;

         ~Environment() = default;

         Environment& operator=(Environment const&) = delete;
         Environment& operator=(Environment&&) = delete;

         ERU_API [[nodiscard]] std::string environment_variable(std::string_view name) const;
         ERU_API void change_environment_variable(std::string_view name, std::string_view value);
         ERU_API void append_environment_variable(std::string_view name, std::string_view value);
   };
}

#endif