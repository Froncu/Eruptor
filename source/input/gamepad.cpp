#include "gamepad.hpp"
#include "services/input_manager/input_manager.hpp"
#include "utility/runtime_assert.hpp"

namespace eru
{
   int Gamepad::user_input_id(ID::InternalValue const id)
   {
      return SDL_GetGamepadPlayerIndexForID(id);
   }

   Gamepad::Gamepad(ID::InternalValue const id)
      : native_gamepad_{
         [](ID::InternalValue const id)
         {
            SDL_Gamepad* native_gamepad{ SDL_OpenGamepad(id) };
            runtime_assert(native_gamepad, "failed to open a Gamepad with ID {} ({})",
               id, SDL_GetError());

            return native_gamepad;
         }(id),
         SDL_CloseGamepad
      }
   {
   }

   ID::InternalValue Gamepad::id() const
   {
      ID::InternalValue const id{ SDL_GetGamepadID(native_gamepad_.get()) };
      runtime_assert(id, "failed to get the ID of a Gamepad ({})",
         SDL_GetError());

      return id;
   }

   int Gamepad::user_input_id() const
   {
      return user_input_id(id());
   }

   void Gamepad::assign_user_input_id(int user_input_id) const
   {
      bool const succeeded{ SDL_SetGamepadPlayerIndex(native_gamepad_.get(), user_input_id) };
      runtime_assert(succeeded, "failed to assign UserInput{} to Gamepad{} ({})",
         user_input_id, id(), SDL_GetError());
   }

   // bool Gamepad::rumble(std::uint16_t const low_frequency, std::uint16_t const high_frequency,
   //    std::uint32_t const duration_milliseconds) const
   // {
   //    if (SDL_RumbleGamepad(native_gamepad_.get(), low_frequency, high_frequency, duration_milliseconds))
   //       return true;
   //
   //    Locator::get<Logger>().warning("failed to rumble Gamepad{} ({})",
   //       id(), SDL_GetError());
   //
   //    return false;
   // }
}