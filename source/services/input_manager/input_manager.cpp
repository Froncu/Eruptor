#include "erupch/erupch.hpp"
#include "input_manager.hpp"

namespace eru
{
   void InputManager::assign_keyboard_mouse(UserInput const& user_input)
   {
      if (keyboard_mouse_user_input_id_ == user_input)
         return;

      if (keyboard_mouse_user_input_id_ not_eq UserInput::INVALID_USER_ID)
         user_input.move_input_strength_if(this->user_input(keyboard_mouse_user_input_id_),
            [](Input const input)
            {
               return std::holds_alternative<MouseButton>(input) or std::holds_alternative<Key>(input);
            });

      keyboard_mouse_user_input_id_ = user_input.id();
   }

   void InputManager::unassign_keyboard_mouse()
   {
      if (keyboard_mouse_user_input_id_ == UserInput::INVALID_USER_ID)
         return;

      this->user_input(keyboard_mouse_user_input_id_).reset_input_strength_if(
         [](Input const input)
         {
            return std::holds_alternative<MouseButton>(input) or std::holds_alternative<Key>(input);
         });

      keyboard_mouse_user_input_id_ = UserInput::INVALID_USER_ID;
   }

   void InputManager::assign_gamepad(UserInput const& target_user_input, Gamepad const& source_gamepad)
   {
      if (target_user_input == source_gamepad.user_input_id())
         return;

      auto const target_gamepad{
         std::ranges::find_if(gamepads_,
            [&target_user_input](Gamepad const& gamepad)
            {
               return target_user_input == gamepad.user_input_id();
            })
      };

      auto const predicate{
         [](Input const input)
         {
            return std::holds_alternative<GamepadButton>(input) or std::holds_alternative<GamepadAxis>(input);
         }
      };

      if (source_gamepad.user_input_id() == UserInput::INVALID_USER_ID)
      {
         if (target_gamepad not_eq gamepads_.end())
         {
            target_user_input.reset_input_strength_if(predicate);
            target_gamepad->assign_user_input_id(UserInput::INVALID_USER_ID);
         }
      }
      else
      {
         UserInput const& source_user_input{ this->user_input(source_gamepad.user_input_id()) };

         if (target_gamepad not_eq gamepads_.end())
         {
            target_user_input.swap_input_strengths_if(source_user_input, predicate);
            target_gamepad->assign_user_input_id(source_user_input.id());
         }
         else
            target_user_input.move_input_strength_if(source_user_input, predicate);
      }

      source_gamepad.assign_user_input_id(target_user_input.id());
   }

   void InputManager::unassign_gamepad(Gamepad const& gamepad)
   {
      if (gamepad.user_input_id() == UserInput::INVALID_USER_ID)
         return;

      this->user_input(gamepad.user_input_id()).reset_input_strength_if(
         [](Input const input)
         {
            return std::holds_alternative<GamepadButton>(input) or std::holds_alternative<GamepadAxis>(input);
         });

      gamepad.assign_user_input_id(UserInput::INVALID_USER_ID);
   }

   void InputManager::change_mouse_sensitivity(float const sensitivity)
   {
      mouse_sensitivity_ = std::max(0.0f, sensitivity);
   }

   UserInput const& InputManager::user_input(int const id)
   {
      if (id == UserInput::INVALID_USER_ID)
         exception("attempted to retrieve an UserInput with invalid user ID");

      auto user_input{ user_inputs_.find(id) };
      if (user_input == user_inputs_.end())
         user_input = user_inputs_.insert(UserInput{ id }).first;

      return *user_input;
   }

   std::vector<Gamepad> const& InputManager::gamepads() const
   {
      return gamepads_;
   }
}