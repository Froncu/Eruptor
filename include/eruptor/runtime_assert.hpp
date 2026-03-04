#ifndef RUNTIME_ASSERT_HPP
#define RUNTIME_ASSERT_HPP

#include "eruptor/constants.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"

namespace eru
{
   template <typename Message>
   constexpr void runtime_assert(bool const condition, Message&& message,
      std::source_location location = std::source_location::current())
   {
      if constexpr (DEBUG_BUILD)
      {
         if (condition)
            return;

         Locator::get<Logger>().error(std::forward<Message>(message), false, std::move(location));
         std::abort();
      }
   }
}

#endif