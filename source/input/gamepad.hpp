#ifndef GAMEPAD_HPP
#define GAMEPAD_HPP

#include "erupch/erupch.hpp"
#include "identifier/id.hpp"
#include "user_input.hpp"
#include "utility/unique_pointer.hpp"

struct SDL_Gamepad;

namespace eru
{
   class Gamepad final : public Referenceable
   {
      friend class InputManager;

      public:
         Gamepad(Gamepad const&) = delete;
         Gamepad(Gamepad&&) = default;

         virtual ~Gamepad() override = default;

         Gamepad& operator=(Gamepad const&) = delete;
         Gamepad& operator=(Gamepad&&) = default;

         [[nodiscard]] ID::InternalValue id() const;
         [[nodiscard]] int user_input_id() const;

      private:
         [[nodiscard]] static int user_input_id(ID::InternalValue id);

         explicit Gamepad(ID::InternalValue id);

         void assign_user_input_id(int user_input_id) const;

         UniquePointer<SDL_Gamepad> native_gamepad_;
   };
}

#endif