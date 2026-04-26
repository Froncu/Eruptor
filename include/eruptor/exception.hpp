#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Exception final : public std::runtime_error
   {
      public:
         ERU_API explicit Exception(std::string_view message, std::source_location source_location = std::source_location::current());
         Exception(Exception const&) = default;
         Exception(Exception&&) = default;

         ~Exception() override = default;

         auto operator=(Exception const&) -> Exception& = default;
         auto operator=(Exception&&) -> Exception& = default;

         [[nodiscard]] ERU_API auto source_location() const -> std::source_location const&;

      private:
         std::source_location source_location_;
   };
}

#endif