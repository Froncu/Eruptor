#ifndef VOID_DELETER_HPP
#define VOID_DELETER_HPP

namespace eru
{
   template <typename Type>
   static auto void_deleter(void* const value) -> void
   {
      delete static_cast<Type* const>(value);
   }
}

#endif