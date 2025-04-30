// Written based on Alx Sathskovsky's LinkedIn post
// (https://www.linkedin.com/pulse/generic-tuple-hashing-modern-c-alex-dathskovsky/)

#ifndef LAZY_HASHER_HPP
#define LAZY_HASHER_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   struct LazyHasher final
   {
      auto operator()(std::size_t hash, auto&&... values) const
      {
         auto lazy_combiner
         {
            [&hash]<typename Type>(Type&& value)
            {
               return hash ^=
                  std::hash<std::decay_t<Type>>{}(value)+
                  0x9e3779b9 +
                  (hash << 6) +
                  (hash >> 2);
            }
         };

         (lazy_combiner(std::forward<decltype(values)>(values)), ...);

         return hash;
      }
   };
}

#endif