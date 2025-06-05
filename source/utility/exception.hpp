#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   template <typename... Arguments>
   [[noreturn]] void exception(std::format_string<Arguments...> const format,
      Arguments&&... arguments)
   {
      throw std::runtime_error(std::format(format, std::forward<Arguments>(arguments)...));
   }

   template <typename Message>
   [[noreturn]] void exception(Message&& message)
   {
      exception("{}", std::forward<Message>(message));
   }
}

#endif