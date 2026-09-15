#pragma once

namespace eru
{
   template<typename Type>
   static auto void_deleter(void* const value) -> void
   {
      delete static_cast<Type* const>(value);
   }
}