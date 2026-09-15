#pragma once

#include <cstdlib>

#include <eruptor/core/locator.hpp>
#include <eruptor/core/logger.hpp>

#ifdef NDEBUG
   #define RUNTIME_ASSERT(...)
#else
   #define RUNTIME_ASSERT(condition, message)\
      if (not (condition))\
      {\
         eru::Locator::get<eru::Logger>().error(message);\
         std::abort();\
      }
#endif