#pragma once

#include <typeindex>

namespace eru
{
   template<typename Type>
   auto type_index() -> std::type_index
   {
      return typeid(Type);
   }
}