#ifndef RUNTIME_ASSERT_HPP
#define RUNTIME_ASSERT_HPP

#ifdef NDEBUG
   #define RUNTIME_ASSERT(...)
#else
   #include "eruptor/logger.hpp"
   #include "eruptor/locator.hpp"
   #include "eruptor/pch.hpp"

   #define RUNTIME_ASSERT(condition, message)\
      if (not (condition))\
      {\
         eru::Locator::get<eru::Logger>().error(message);\
         std::abort();\
      }
#endif

#endif