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

         Exception& operator=(Exception const&) = default;
         Exception& operator=(Exception&&) = default;

         [[nodiscard]] ERU_API std::source_location const& source_location() const;

      private:
         std::source_location source_location_;
   };
}

#endif