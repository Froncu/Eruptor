#ifndef INPUT_CONVERSIONS_HPP
#define INPUT_CONVERSIONS_HPP

#include "input.hpp"

namespace eru
{
   [[nodiscard]] MouseButton convert_sdl_mouse_button(Uint8 mouse_button);
   [[nodiscard]] Key convert_sdl_key_code(SDL_Keycode key_code);
   [[nodiscard]] GamepadButton convert_sdl_controller_button(Uint8 button);
   [[nodiscard]] GamepadAxis convert_sdl_controller_axis(Uint8 axis, Sint16 value);
   [[nodiscard]] std::pair<GamepadAxis, float> convert_sdl_controller_axis_value(Uint8 axis, Sint16 value);
}

#endif