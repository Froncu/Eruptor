#include "input.hpp"
#include "input_conversions.hpp"

namespace eru
{
   Key convert_sdl_key_code(SDL_Keycode const key_code)
   {
      switch (key_code)
      {
         case SDLK_RETURN:
         case SDLK_ESCAPE:
         case SDLK_BACKSPACE:
         case SDLK_TAB:
         case SDLK_SPACE:
         case SDLK_EXCLAIM:
         case SDLK_DBLAPOSTROPHE:
         case SDLK_HASH:
         case SDLK_PERCENT:
         case SDLK_DOLLAR:
         case SDLK_AMPERSAND:
         case SDLK_APOSTROPHE:
         case SDLK_LEFTPAREN:
         case SDLK_RIGHTPAREN:
         case SDLK_ASTERISK:
         case SDLK_PLUS:
         case SDLK_COMMA:
         case SDLK_MINUS:
         case SDLK_PERIOD:
         case SDLK_SLASH:
         case SDLK_0:
         case SDLK_1:
         case SDLK_2:
         case SDLK_3:
         case SDLK_4:
         case SDLK_5:
         case SDLK_6:
         case SDLK_7:
         case SDLK_8:
         case SDLK_9:
         case SDLK_COLON:
         case SDLK_SEMICOLON:
         case SDLK_LESS:
         case SDLK_EQUALS:
         case SDLK_GREATER:
         case SDLK_QUESTION:
         case SDLK_AT:
         case SDLK_LEFTBRACKET:
         case SDLK_BACKSLASH:
         case SDLK_RIGHTBRACKET:
         case SDLK_CARET:
         case SDLK_UNDERSCORE:
         case SDLK_GRAVE:
         case SDLK_A:
         case SDLK_B:
         case SDLK_C:
         case SDLK_D:
         case SDLK_E:
         case SDLK_F:
         case SDLK_G:
         case SDLK_H:
         case SDLK_I:
         case SDLK_J:
         case SDLK_K:
         case SDLK_L:
         case SDLK_M:
         case SDLK_N:
         case SDLK_O:
         case SDLK_P:
         case SDLK_Q:
         case SDLK_R:
         case SDLK_S:
         case SDLK_T:
         case SDLK_U:
         case SDLK_V:
         case SDLK_W:
         case SDLK_X:
         case SDLK_Y:
         case SDLK_Z:
         case SDLK_DELETE:
            return static_cast<Key>(key_code);

         case SDLK_CAPSLOCK:
            return Key::CAPS_LOCK;

         case SDLK_F1:
            return Key::F1;

         case SDLK_F2:
            return Key::F2;

         case SDLK_F3:
            return Key::F3;

         case SDLK_F4:
            return Key::F4;

         case SDLK_F5:
            return Key::F5;

         case SDLK_F6:
            return Key::F6;

         case SDLK_F7:
            return Key::F7;

         case SDLK_F8:
            return Key::F8;

         case SDLK_F9:
            return Key::F9;

         case SDLK_F10:
            return Key::F10;

         case SDLK_F11:
            return Key::F11;

         case SDLK_F12:
            return Key::F12;

         case SDLK_PRINTSCREEN:
            return Key::PRINT_SCREEN;

         case SDLK_SCROLLLOCK:
            return Key::SCROLL_LOCK;

         case SDLK_PAUSE:
            return Key::PAUSE;

         case SDLK_INSERT:
            return Key::INSERT;

         case SDLK_HOME:
            return Key::HOME;

         case SDLK_PAGEUP:
            return Key::PAGE_UP;

         case SDLK_END:
            return Key::END;

         case SDLK_PAGEDOWN:
            return Key::PAGE_DOWN;

         case SDLK_RIGHT:
            return Key::RIGHT;

         case SDLK_LEFT:
            return Key::LEFT;

         case SDLK_DOWN:
            return Key::DOWN;

         case SDLK_UP:
            return Key::UP;

         case SDLK_NUMLOCKCLEAR:
            return Key::NUM_LOCK_CLEAR;

         case SDLK_KP_DIVIDE:
            return Key::KEY_PAD_DIVIDE;

         case SDLK_KP_MULTIPLY:
            return Key::KEY_PAD_MULTIPLY;

         case SDLK_KP_MINUS:
            return Key::KEY_PAD_MINUS;

         case SDLK_KP_PLUS:
            return Key::KEY_PAD_PLUS;

         case SDLK_KP_ENTER:
            return Key::KEY_PAD_ENTER;

         case SDLK_KP_1:
            return Key::KEY_PAD_NUMBER_1;

         case SDLK_KP_2:
            return Key::KEY_PAD_NUMBER_2;

         case SDLK_KP_3:
            return Key::KEY_PAD_NUMBER_3;

         case SDLK_KP_4:
            return Key::KEY_PAD_NUMBER_4;

         case SDLK_KP_5:
            return Key::KEY_PAD_NUMBER_5;

         case SDLK_KP_6:
            return Key::KEY_PAD_NUMBER_6;

         case SDLK_KP_7:
            return Key::KEY_PAD_NUMBER_7;

         case SDLK_KP_8:
            return Key::KEY_PAD_NUMBER_8;

         case SDLK_KP_9:
            return Key::KEY_PAD_NUMBER_9;

         case SDLK_KP_0:
            return Key::KEY_PAD_NUMBER_0;

         case SDLK_KP_PERIOD:
            return Key::KEY_PAD_PERIOD;

         case SDLK_APPLICATION:
            return Key::APPLICATION;

         case SDLK_POWER:
            return Key::POWER;

         case SDLK_KP_EQUALS:
            return Key::KEY_PAD_EQUALS;

         case SDLK_F13:
            return Key::F13;

         case SDLK_F14:
            return Key::F14;

         case SDLK_F15:
            return Key::F15;

         case SDLK_F16:
            return Key::F16;

         case SDLK_F17:
            return Key::F17;

         case SDLK_F18:
            return Key::F18;

         case SDLK_F19:
            return Key::F19;

         case SDLK_F20:
            return Key::F20;

         case SDLK_F21:
            return Key::F21;

         case SDLK_F22:
            return Key::F22;

         case SDLK_F23:
            return Key::F23;

         case SDLK_F24:
            return Key::F24;

         case SDLK_EXECUTE:
            return Key::EXECUTE;

         case SDLK_HELP:
            return Key::HELP;

         case SDLK_MENU:
            return Key::MENU;

         case SDLK_SELECT:
            return Key::SELECT;

         case SDLK_STOP:
            return Key::STOP;

         case SDLK_AGAIN:
            return Key::AGAIN;

         case SDLK_UNDO:
            return Key::UNDO;

         case SDLK_CUT:
            return Key::CUT;

         case SDLK_COPY:
            return Key::COPY;

         case SDLK_PASTE:
            return Key::PASTE;

         case SDLK_FIND:
            return Key::FIND;

         case SDLK_MUTE:
            return Key::MUTE;

         case SDLK_VOLUMEUP:
            return Key::VOLUME_UP;

         case SDLK_VOLUMEDOWN:
            return Key::VOLUME_DOWN;

         case SDLK_MEDIA_NEXT_TRACK:
            return Key::MEDIA_NEXT;

         case SDLK_MEDIA_PREVIOUS_TRACK:
            return Key::MEDIA_PREVIOUS;

         case SDLK_MEDIA_STOP:
            return Key::MEDIA_STOP;

         case SDLK_MEDIA_PLAY:
            return Key::MEDIA_PLAY;

         case SDLK_MEDIA_SELECT:
            return Key::MEDIA_SELECT;

         case SDLK_MEDIA_EJECT:
            return Key::EJECT;

         case SDLK_MEDIA_REWIND:
            return Key::MEDIA_REWIND;

         case SDLK_MEDIA_FAST_FORWARD:
            return Key::MEDIA_FAST_FORWARD;

         case SDLK_KP_COMMA:
            return Key::KEY_PAD_COMMA;

         case SDLK_KP_EQUALSAS400:
            return Key::KEY_PAD_EQUALS_AS_400;

         case SDLK_ALTERASE:
            return Key::ALTER_ASE;

         case SDLK_SYSREQ:
            return Key::SYSTEM_REQUEST;

         case SDLK_CANCEL:
            return Key::CANCEL;

         case SDLK_CLEAR:
            return Key::CLEAR;

         case SDLK_PRIOR:
            return Key::PRIOR;

         case SDLK_RETURN2:
            return Key::RETURN_2;

         case SDLK_SEPARATOR:
            return Key::SEPARATOR;

         case SDLK_OUT:
            return Key::OUT;

         case SDLK_OPER:
            return Key::OPER;

         case SDLK_CLEARAGAIN:
            return Key::CLEAR_AGAIN;

         case SDLK_CRSEL:
            return Key::CR_SEL;

         case SDLK_EXSEL:
            return Key::EX_SEL;

         case SDLK_KP_00:
            return Key::KEY_PAD_NUMBER_00;

         case SDLK_KP_000:
            return Key::KEY_PAD_NUMBER_000;

         case SDLK_THOUSANDSSEPARATOR:
            return Key::THOUSANDS_SEPARATOR;

         case SDLK_DECIMALSEPARATOR:
            return Key::DECIMAL_SEPARATOR;

         case SDLK_CURRENCYUNIT:
            return Key::CURRENCY_UNIT;

         case SDLK_CURRENCYSUBUNIT:
            return Key::CURRENCY_SUB_UNIT;

         case SDLK_KP_LEFTPAREN:
            return Key::KEY_PAD_LEFT_PARENTHESIS;

         case SDLK_KP_RIGHTPAREN:
            return Key::KEY_PAD_RIGHT_PARENTHESIS;

         case SDLK_KP_LEFTBRACE:
            return Key::KEY_PAD_LEFT_BRACE;

         case SDLK_KP_RIGHTBRACE:
            return Key::KEY_PAD_RIGHT_BRACE;

         case SDLK_KP_TAB:
            return Key::KEY_PAD_TAB;

         case SDLK_KP_BACKSPACE:
            return Key::KEY_PAD_BACKSPACE;

         case SDLK_KP_A:
            return Key::KEY_PAD_A;

         case SDLK_KP_B:
            return Key::KEY_PAD_B;

         case SDLK_KP_C:
            return Key::KEY_PAD_C;

         case SDLK_KP_D:
            return Key::KEY_PAD_D;

         case SDLK_KP_E:
            return Key::KEY_PAD_E;

         case SDLK_KP_F:
            return Key::KEY_PAD_F;

         case SDLK_KP_XOR:
            return Key::KEY_PAD_XOR;

         case SDLK_KP_POWER:
            return Key::KEY_PAD_POWER;

         case SDLK_KP_PERCENT:
            return Key::KEY_PAD_PERCENT;

         case SDLK_KP_LESS:
            return Key::KEY_PAD_LESS;

         case SDLK_KP_GREATER:
            return Key::KEY_PAD_GREATER;

         case SDLK_KP_AMPERSAND:
            return Key::KEY_PAD_AMPERSAND;

         case SDLK_KP_DBLAMPERSAND:
            return Key::KEY_PAD_DOUBLE_AMPERSAND;

         case SDLK_KP_VERTICALBAR:
            return Key::KEY_PAD_VERTICALBAR;

         case SDLK_KP_DBLVERTICALBAR:
            return Key::KEY_PAD_DOUBLEB_VERTICAL_BAR;

         case SDLK_KP_COLON:
            return Key::KEY_PAD_COLON;

         case SDLK_KP_HASH:
            return Key::KEY_PAD_HASH;

         case SDLK_KP_SPACE:
            return Key::KEY_PAD_SPACE;

         case SDLK_KP_AT:
            return Key::KEY_PAD_AT;

         case SDLK_KP_EXCLAM:
            return Key::KEY_PAD_EXCLAMATION_MARK;

         case SDLK_KP_MEMSTORE:
            return Key::KEY_PAD_MEMORY_STORE;

         case SDLK_KP_MEMRECALL:
            return Key::KEY_PAD_MEMORY_RECALL;

         case SDLK_KP_MEMCLEAR:
            return Key::KEY_PAD_MEMORY_CLEAR;

         case SDLK_KP_MEMADD:
            return Key::KEY_PAD_MEMORY_ADD;

         case SDLK_KP_MEMSUBTRACT:
            return Key::KEY_PAD_MEMORY_SUBTRACT;

         case SDLK_KP_MEMMULTIPLY:
            return Key::KEY_PAD_MEMORY_MULTIPLY;

         case SDLK_KP_MEMDIVIDE:
            return Key::KEY_PAD_MEMORY_DIVIDE;

         case SDLK_KP_PLUSMINUS:
            return Key::KEY_PAD_PLUS_MINUS;

         case SDLK_KP_CLEAR:
            return Key::KEY_PAD_CLEAR;

         case SDLK_KP_CLEARENTRY:
            return Key::KEY_PAD_CLEAR_ENTRY;

         case SDLK_KP_BINARY:
            return Key::KEY_PAD_BINARY;

         case SDLK_KP_OCTAL:
            return Key::KEY_PAD_OCTAL;

         case SDLK_KP_DECIMAL:
            return Key::KEY_PAD_DECIMAL;

         case SDLK_KP_HEXADECIMAL:
            return Key::KEY_PAD_HEXADECIMAL;

         case SDLK_LCTRL:
            return Key::LEFT_CTRL;

         case SDLK_LSHIFT:
            return Key::LEFT_SHIFT;

         case SDLK_LALT:
            return Key::LEFT_ALT;

         case SDLK_LGUI:
            return Key::LEFT_GUI;

         case SDLK_RCTRL:
            return Key::RIGHT_CTRL;

         case SDLK_RSHIFT:
            return Key::RIGHT_SHIFT;

         case SDLK_RALT:
            return Key::RIGHT_ALT;

         case SDLK_RGUI:
            return Key::RIGHT_GUI;

         case SDLK_MODE:
            return Key::MODE;

         case SDLK_AC_SEARCH:
            return Key::AC_SEARCH;

         case SDLK_AC_HOME:
            return Key::AC_HOME;

         case SDLK_AC_BACK:
            return Key::AC_BACK;

         case SDLK_AC_FORWARD:
            return Key::AC_FORWARD;

         case SDLK_AC_STOP:
            return Key::AC_STOP;

         case SDLK_AC_REFRESH:
            return Key::AC_REFRESH;

         case SDLK_AC_BOOKMARKS:
            return Key::AC_BOOKMARKS;

         case SDLK_SLEEP:
            return Key::SLEEP;

         case SDLK_SOFTLEFT:
            return Key::SOFT_LEFT;

         case SDLK_SOFTRIGHT:
            return Key::SOFT_RIGHT;

         case SDLK_CALL:
            return Key::CALL;

         case SDLK_ENDCALL:
            return Key::END_CALL;

         default:
            return Key::UNKNOWN;
      }
   }

   GamepadButton convert_sdl_controller_button(Uint8 const button)
   {
      switch (button)
      {
         case SDL_GAMEPAD_BUTTON_SOUTH:
            return GamepadButton::SOUTH;

         case SDL_GAMEPAD_BUTTON_EAST:
            return GamepadButton::EAST;

         case SDL_GAMEPAD_BUTTON_WEST:
            return GamepadButton::WEST;

         case SDL_GAMEPAD_BUTTON_NORTH:
            return GamepadButton::NORTH;

         case SDL_GAMEPAD_BUTTON_BACK:
            return GamepadButton::BACK;

         case SDL_GAMEPAD_BUTTON_GUIDE:
            return GamepadButton::GUIDE;

         case SDL_GAMEPAD_BUTTON_START:
            return GamepadButton::START;

         case SDL_GAMEPAD_BUTTON_LEFT_STICK:
            return GamepadButton::LEFT_STICK;

         case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
            return GamepadButton::RIGHT_STICK;

         case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
            return GamepadButton::LEFT_SHOULDER;

         case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
            return GamepadButton::RIGHT_SHOULDER;

         case SDL_GAMEPAD_BUTTON_DPAD_UP:
            return GamepadButton::UP;

         case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
            return GamepadButton::DOWN;

         case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
            return GamepadButton::LEFT;

         case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
            return GamepadButton::RIGHT;

         case SDL_GAMEPAD_BUTTON_MISC1:
            return GamepadButton::MISCELLANEOUS;

         case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1:
            return GamepadButton::RIGHT_PADDLE_1;

         case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1:
            return GamepadButton::LEFT_PADDLE_1;

         case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2:
            return GamepadButton::RIGHT_PADDLE_2;

         case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2:
            return GamepadButton::LEFT_PADDLE_2;

         case SDL_GAMEPAD_BUTTON_TOUCHPAD:
            return GamepadButton::TOUCHPAD;

         default:
            return GamepadButton::UNKNOWN;
      }
   }

   GamepadAxis convert_sdl_controller_axis(Uint8 const axis, Sint16 const value)
   {
      switch (axis)
      {
         case SDL_GAMEPAD_AXIS_LEFTX:
            return value >= 0 ? GamepadAxis::LEFT_STICK_EAST : GamepadAxis::LEFT_STICK_WEST;

         case SDL_GAMEPAD_AXIS_LEFTY:
            return value >= 0 ? GamepadAxis::LEFT_STICK_SOUTH : GamepadAxis::LEFT_STICK_NORTH;

         case SDL_GAMEPAD_AXIS_RIGHTX:
            return value >= 0 ? GamepadAxis::RIGHT_STICK_EAST : GamepadAxis::RIGHT_STICK_WEST;

         case SDL_GAMEPAD_AXIS_RIGHTY:
            return value >= 0 ? GamepadAxis::RIGHT_STICK_SOUTH : GamepadAxis::RIGHT_STICK_NORTH;

         case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
            return GamepadAxis::LEFT_TRIGGER;

         case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
            return GamepadAxis::RIGHT_TRIGGER;

         default:
            return GamepadAxis::UNKNOWN;
      }
   }

   std::pair<GamepadAxis, float> convert_sdl_controller_axis_value(Uint8 const axis, Sint16 const value)
   {
      auto constexpr highest_axis_value{ std::numeric_limits<decltype(value)>::max() };
      auto constexpr lowest_axis_value{ std::numeric_limits<decltype(value)>::lowest() };
      return {
         convert_sdl_controller_axis(axis, value),
         static_cast<float>(value) / (value >= 0 ? highest_axis_value : lowest_axis_value)
      };
   }
}