#ifndef API_HPP
#define API_HPP

#ifdef ERU_SHARED
   #ifdef _MSC_VER
      #ifdef ERU_FRAMEWORK_LEVEL
         #define ERU_API __declspec(dllexport)
      #else
         #define ERU_API __declspec(dllimport)
      #endif
   #else
      #define ERU_API __attribute__((__visibility__("default")))
   #endif
#else
   #define ERU_API
#endif

#endif