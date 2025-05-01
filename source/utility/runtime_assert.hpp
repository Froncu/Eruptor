#ifndef ASSERT_HPP
#define ASSERT_HPP

#include "constants.hpp"
#include "erupch/erupch.hpp"

namespace eru
{
   template <typename... Arguments>
   void runtime_assert([[maybe_unused]] bool const condition,
      [[maybe_unused]] spdlog::format_string_t<Arguments...> const format,
      [[maybe_unused]] Arguments&&... arguments)
   {
      if constexpr (constants::DEBUG)
      {
         if (condition)
            return;

         spdlog::error(format, std::forward<Arguments>(arguments)...);
         std::abort();
      }
   }

   template <typename Message>
   void runtime_assert(bool const condition, Message&& message)
   {
      runtime_assert(condition, "{}", std::forward<Message>(message));
   }
}

#endif