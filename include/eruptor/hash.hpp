#ifndef HASH_HPP
#define HASH_HPP

#include "eruptor/pch.hpp"

namespace eru
{
   template <typename Value>
   auto hash(Value const& value) -> std::size_t
   {
      return std::hash<Value>{}(value);
   }
}

#endif