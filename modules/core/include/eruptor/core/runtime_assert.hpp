#pragma once

#include <exception>
#include <utility>

#include <eruptor/core/locator.hpp>
#include <eruptor/core/logger.hpp>

#ifdef NDEBUG
   #define RUNTIME_ASSERT(condition, message)\
      do\
      {\
         std::ignore = sizeof((condition));\
         std::ignore = sizeof((message));\
      }\
      while (false)
#else
   #define RUNTIME_ASSERT(condition, message)\
      do\
      {\
         if (not (condition))\
         {\
            eru::Locator::get<eru::Logger>().error(message);\
            std::terminate();\
         }\
      }\
      while (false)
#endif