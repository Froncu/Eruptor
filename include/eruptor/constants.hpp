#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace eru
{
   #ifdef NDEBUG
   auto constexpr DEBUG_BUILD{ false };
   #else
   auto constexpr DEBUG_BUILD{ true };
   #endif

   #ifdef ERU_FRAMEWORK_LEVEL
   auto constexpr FRAMEWORK_LEVEL{ true };
   std::string_view constexpr COMPILE_SOURCE_PATH{ ERU_COMPILE_SOURCE_PATH };
   #else
   auto constexpr FRAMEWORK_LEVEL{ false };
   #endif

   #ifdef __MINGW32__
   auto constexpr MINGW{ true };
   #else
   auto constexpr MINGW{ false };
   #endif
}

#endif