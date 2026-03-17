#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace eru
{
   #ifdef NDEBUG
   inline constexpr auto DEBUG_BUILD{ false };
   #else
   inline constexpr auto DEBUG_BUILD{ true };
   #endif

   #ifdef ERU_FRAMEWORK_LEVEL
   inline constexpr auto FRAMEWORK_LEVEL{ true };
   #else
   inline constexpr auto FRAMEWORK_LEVEL{ false };
   #endif

   #ifdef ERU_FRAMEWORK_LEVEL
   inline constexpr std::string_view COMPILE_SOURCE_PATH{ ERU_COMPILE_SOURCE_PATH };
   #endif
}

#endif