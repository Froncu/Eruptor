#pragma once

namespace eru
{
   template<typename Value>
   auto hash(Value const& value) -> std::size_t
   {
      return std::hash<Value>{}(value);
   }
}