#ifndef HASH_HPP
#define HASH_HPP

#include "eruptor/pch.hpp"

namespace eru
{
   template <typename Value>
   std::size_t hash(Value const& value)
   {
      return std::hash<Value>{}(value);
   }
}

#endif