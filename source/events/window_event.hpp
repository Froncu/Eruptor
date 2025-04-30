#ifndef WINDOW_EVENT_HPP
#define WINDOW_EVENT_HPP

#include "erupch/erupch.hpp"
#include "identifier/id.hpp"

namespace eru
{
   struct WindowCloseEvent final
   {
      ID::InternalValue const id;
   };

   struct WindowResizeEvent final
   {
      ID::InternalValue const id;
      vk::Extent2D const extent;
   };

   using WindowEvent = std::variant<
      WindowCloseEvent,
      WindowResizeEvent>;
}

#endif