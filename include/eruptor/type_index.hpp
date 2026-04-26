#ifndef TYPE_INDEX_HPP
#define TYPE_INDEX_HPP

#include "eruptor/pch.hpp"

namespace eru
{
   template <typename Type>
   auto type_index() -> std::type_index
   {
      return typeid(Type);
   }
}

#endif