#include "eruptor/exception.hpp"

namespace eru
{
   Exception::Exception(std::string_view message, std::source_location source_location)
      : exception{ message.data() }
      , source_location_{ std::move(source_location) }
   {
   }

   std::source_location const& Exception::source_location() const
   {
      return source_location_;
   }
}