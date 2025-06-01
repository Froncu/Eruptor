#include "erupch/erupch.hpp"
#include "input/input_conversions.hpp"
#include "system_event_dispatcher.hpp"

namespace eru
{
   void SystemEventDispatcher::poll_events()
   {
      bool did_mouse_move_now{};
      SDL_Event native_event;
      while (SDL_PollEvent(&native_event))
         switch (native_event.type)
         {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
               window_event.notify(WindowCloseEvent{ .id{ native_event.window.windowID } });
               break;

            case SDL_EVENT_WINDOW_RESIZED:
               window_event.notify(WindowResizeEvent{
                  .extent{
                     .width{ static_cast<std::uint32_t>(native_event.window.data1) },
                     .height{ static_cast<std::uint32_t>(native_event.window.data2) }
                  }
               });
               break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
               mouse_input_event.notify(MouseButtonDownEvent{
                  .button{ convert_sdl_mouse_button(native_event.button.button) },
               });
               break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
               mouse_input_event.notify(MouseButtonUpEvent{
                  .button{ convert_sdl_mouse_button(native_event.button.button) },
               });
               break;

            case SDL_EVENT_MOUSE_MOTION:
               if (native_event.motion.xrel)
                  mouse_input_event.notify(MouseAxisEvent{
                     .axis{ native_event.motion.xrel > 0.0 ? MouseAxis::EAST : MouseAxis::WEST },
                     .value{ std::abs(native_event.motion.xrel) }
                  });

               if (native_event.motion.yrel)
                  mouse_input_event.notify(MouseAxisEvent{
                     .axis{ native_event.motion.yrel > 0.0 ? MouseAxis::SOUTH : MouseAxis::NORTH },
                     .value{ std::abs(native_event.motion.yrel) }
                  });

               did_mouse_move_now = true;
               did_mouse_move_previously_ = true;

            case SDL_EVENT_KEY_DOWN:
               key_event.notify(KeyDownEvent{ .key{ convert_sdl_key_code(native_event.key.key) } });
               break;

            case SDL_EVENT_KEY_UP:
               key_event.notify(KeyUpEvent{ .key{ convert_sdl_key_code(native_event.key.key) } });
               break;

            case SDL_EVENT_GAMEPAD_ADDED:
               gamepad_connection_event.notify(GamepadConnectedEvent{ .id{ native_event.gdevice.which } });
               break;

            case SDL_EVENT_GAMEPAD_REMOVED:
               gamepad_connection_event.notify(GamepadDisconnectedEvent{ .id{ native_event.gdevice.which } });
               previous_gamepad_stick_values_.erase(native_event.gdevice.which);
               break;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
               gamepad_input_event.notify(GamepadButtonDownEvent{
                  .id{ native_event.gbutton.which },
                  .button{ convert_sdl_controller_button(native_event.gbutton.button) }
               });
               break;

            case SDL_EVENT_GAMEPAD_BUTTON_UP:
               gamepad_input_event.notify(GamepadButtonUpEvent{
                  .id{ native_event.gbutton.which },
                  .button{ convert_sdl_controller_button(native_event.gbutton.button) }
               });
               break;

            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            {
               std::int16_t* previous_stick_value;
               auto& [left_stick_x, left_stick_y, right_stick_x, right_stick_y]{
                  previous_gamepad_stick_values_[native_event.gaxis.which]
               };

               switch (native_event.gaxis.axis)
               {
                  case SDL_GAMEPAD_AXIS_LEFTX:
                     previous_stick_value = &left_stick_x;
                     break;

                  case SDL_GAMEPAD_AXIS_LEFTY:
                     previous_stick_value = &left_stick_y;
                     break;

                  case SDL_GAMEPAD_AXIS_RIGHTX:
                     previous_stick_value = &right_stick_x;
                     break;

                  case SDL_GAMEPAD_AXIS_RIGHTY:
                     previous_stick_value = &right_stick_y;
                     break;

                  default:
                     previous_stick_value = nullptr;
                     break;
               }

               if (previous_stick_value)
               {
                  if ((*previous_stick_value < 0 and native_event.gaxis.value > 0) or
                     (*previous_stick_value > 0 and native_event.gaxis.value < 0))
                     gamepad_input_event.notify(GamepadAxisEvent{
                        .id{ native_event.gaxis.which },
                        .axis{ convert_sdl_controller_axis(native_event.gaxis.axis, -native_event.gaxis.value) },
                        .value{}
                     });

                  *previous_stick_value = native_event.gaxis.value;
               }

               auto&& [axis, value]{ convert_sdl_controller_axis_value(native_event.gaxis.axis, native_event.gaxis.value) };
               gamepad_input_event.notify(GamepadAxisEvent{
                  .id{ native_event.gaxis.which },
                  .axis{ axis },
                  .value{ value }
               });
            }

            default:
               break;
         }

      if (not did_mouse_move_now and did_mouse_move_previously_)
      {
         mouse_input_event.notify(MouseAxisEvent{ .axis{ MouseAxis::EAST }, .value{} });
         mouse_input_event.notify(MouseAxisEvent{ .axis{ MouseAxis::WEST }, .value{} });
         mouse_input_event.notify(MouseAxisEvent{ .axis{ MouseAxis::NORTH }, .value{} });
         mouse_input_event.notify(MouseAxisEvent{ .axis{ MouseAxis::SOUTH }, .value{} });
         did_mouse_move_previously_ = false;
      }
   }
}