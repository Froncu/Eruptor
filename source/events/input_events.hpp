#ifndef INPUT_EVENT_HPP
#define INPUT_EVENT_HPP

#include "erupch/erupch.hpp"
#include "identifier/id.hpp"
#include "input/input.hpp"

namespace eru
{
   struct MouseButtonDownEvent final
   {
      MouseButton const button;
   };

   struct MouseButtonUpEvent final
   {
      MouseButton const button;
   };

   struct KeyDownEvent final
   {
      Key const key;
   };

   struct KeyUpEvent final
   {
      Key const key;
   };

   struct GamepadConnectedEvent final
   {
      ID::InternalValue const id;
   };

   struct GamepadDisconnectedEvent final
   {
      ID::InternalValue const id;
   };

   struct GamepadButtonDownEvent final
   {
      ID::InternalValue const id;
      GamepadButton const button;
   };

   struct GamepadButtonUpEvent final
   {
      ID::InternalValue const id;
      GamepadButton const button;
   };

   struct GamepadAxisEvent final
   {
      ID::InternalValue const id;
      GamepadAxis const axis;
      double const value;
   };

   using MouseButtonEvent = std::variant<
      MouseButtonDownEvent,
      MouseButtonUpEvent>;

   using KeyEvent = std::variant<
      KeyDownEvent,
      KeyUpEvent>;

   using GamepadConnectionEvent = std::variant<
      GamepadConnectedEvent,
      GamepadDisconnectedEvent>;

   using GamepadInputEvent = std::variant<
      GamepadButtonDownEvent,
      GamepadButtonUpEvent,
      GamepadAxisEvent>;
}

#endif