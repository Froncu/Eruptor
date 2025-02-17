#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace eru::constants
{
   #ifdef NDEBUG
   auto constexpr DEBUG{ false };
   #else
   auto constexpr DEBUG{ true };
   #endif
};

#endif