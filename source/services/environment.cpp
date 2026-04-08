#include "eruptor/environment.hpp"

namespace eru
{
   Environment::Environment(Locator::ConstructionKey)
   {
   }

   std::string Environment::environment_variable(std::string_view const name) const
   {
      #ifdef _WIN32
      char* value;
      _dupenv_s(&value, nullptr, name.data());
      std::string result{ value ? value : "" };
      free(value);
      return result;
      #else
      return getenv(name.data());
      #endif
   }

   void Environment::change_environment_variable(std::string_view const name, std::string_view const value)
   {
      #ifdef _WIN32
      _putenv_s(name.data(), value.data());
      #else
      setenv(name.data(), value.data(), 1);
      #endif
   }

   void Environment::append_environment_variable(std::string_view name, std::string_view value)
   {
      #ifdef _WIN32
      constexpr char separator = ';';
      #else
      constexpr char separator = ':';
      #endif

      std::string current{ environment_variable(name) };
      change_environment_variable(name,
         current.empty()
            ? value
            : std::move(current) + separator + value.data());
   }
}