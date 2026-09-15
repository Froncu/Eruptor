#pragma once

#include <type_traits>

namespace eru
{
   template<typename...>
   static auto constexpr IS_UNIQUE{ true };

   template<typename Head, typename... Tail>
   static auto constexpr IS_UNIQUE<Head, Tail...>
   {
      (not std::is_same_v<Head, Tail> and ...) and IS_UNIQUE<Tail...>
   };
}