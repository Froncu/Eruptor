#ifndef TYPE_INDEX_HPP
#define TYPE_INDEX_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   template <typename Type>
   std::type_index type_index()
   {
      return std::type_index(typeid(Type));
   }
}

#endif