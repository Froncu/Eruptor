#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include "erupch/erupch.hpp"
#include "events/input_events.hpp"
#include "events/observer/event_listener.hpp"
#include "input/gamepad.hpp"
#include "input/user_input.hpp"
#include "services/locator.hpp"
#include "services/system_event_dispatcher/system_event_dispatcher.hpp"
#include "utility/variant_visitor.hpp"

namespace eru
{
   class InputManager final : public Referenceable
   {
      public:
         InputManager() = default;
         InputManager(InputManager const&) = delete;
         InputManager(InputManager&&) = default;

         virtual ~InputManager() override = default;

         InputManager& operator=(InputManager const&) = delete;
         InputManager& operator=(InputManager&&) = default;

         void assign_keyboard_mouse(UserInput const& user_input);
         void unassign_keyboard_mouse();
         void assign_gamepad(UserInput const& user_input, Gamepad const& gamepad);
         void unassign_gamepad(Gamepad const& gamepad);

         void change_mouse_sensitivity(float sensitivity);

         [[nodiscard]] UserInput const& user_input(int id);
         [[nodiscard]] std::vector<Gamepad> const& gamepads() const;

         EventDispatcher<Gamepad const> gamepad_connected_event{};
         EventDispatcher<Gamepad const> gamepad_disconnected_event{};

         EventListener<MouseInputEvent const> on_mouse_event
         {
            VariantVisitor
            {
               [smart_this = Reference{ this }](MouseButtonDownEvent const& event)
               {
                  if (smart_this->keyboard_mouse_user_input_id_ == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(smart_this->keyboard_mouse_user_input_id_).change_input_strength(event.button, 1.0f);
                  return true;
               },

               [smart_this = Reference{ this }](MouseButtonUpEvent const& event)
               {
                  if (smart_this->keyboard_mouse_user_input_id_ == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(smart_this->keyboard_mouse_user_input_id_).change_input_strength(event.button, 0.0f);
                  return true;
               },

               [smart_this = Reference{ this }](MouseAxisEvent const& event)
               {
                  if (smart_this->keyboard_mouse_user_input_id_ == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(smart_this->keyboard_mouse_user_input_id_).change_input_strength(event.axis,
                     smart_this->mouse_sensitivity_ * event.value);
                  return true;
               }
            },
            Locator::get<SystemEventDispatcher>().mouse_input_event
         };

         EventListener<KeyEvent const> on_key_event
         {
            VariantVisitor
            {
               [smart_this = Reference{ this }](KeyDownEvent const& event)
               {
                  if (smart_this->keyboard_mouse_user_input_id_ == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(smart_this->keyboard_mouse_user_input_id_).change_input_strength(event.key, 1.0f);
                  return true;
               },

               [smart_this = Reference{ this }](KeyUpEvent const& event)
               {
                  if (smart_this->keyboard_mouse_user_input_id_ == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(smart_this->keyboard_mouse_user_input_id_).change_input_strength(event.key, 0.0f);
                  return true;
               }
            },
            Locator::get<SystemEventDispatcher>().key_event
         };

         EventListener<GamepadConnectionEvent const> on_gamepad_connection_event
         {
            VariantVisitor
            {
               [smart_this = Reference{ this }](GamepadConnectedEvent const& event)
               {
                  smart_this->gamepad_connected_event.notify(smart_this->gamepads_.emplace_back(Gamepad{ event.id }));
                  return true;
               },

               [smart_this = Reference{ this }](GamepadDisconnectedEvent const& event)
               {
                  std::erase_if(smart_this->gamepads_,
                     [&event](Gamepad const& gamepad)
                     {
                        return gamepad.id() == event.id;
                     });
                  return true;
               }
            },
            Locator::get<SystemEventDispatcher>().gamepad_connection_event
         };

         EventListener<GamepadInputEvent const> on_gamepad_input_event
         {
            VariantVisitor
            {
               [smart_this = Reference{ this }](GamepadButtonDownEvent const& event)
               {
                  int const user_input_id{ Gamepad::user_input_id(event.id) };
                  if (user_input_id == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(user_input_id).change_input_strength(event.button, 1.0f);
                  return true;
               },

               [smart_this = Reference{ this }](GamepadButtonUpEvent const& event)
               {
                  int const user_input_id{ Gamepad::user_input_id(event.id) };
                  if (user_input_id == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(user_input_id).change_input_strength(event.button, 0.0f);
                  return true;
               },

               [smart_this = Reference{ this }](GamepadAxisEvent const& event)
               {
                  int const user_input_id{ Gamepad::user_input_id(event.id) };
                  if (user_input_id == UserInput::INVALID_USER_ID)
                     return false;

                  smart_this->user_input(user_input_id).change_input_strength(event.axis, event.value);
                  return true;
               }
            },
            Locator::get<SystemEventDispatcher>().gamepad_input_event
         };

      private:
         std::unordered_set<UserInput> user_inputs_{};
         std::vector<Gamepad> gamepads_{};
         float mouse_sensitivity_{ 1.0f };
         int keyboard_mouse_user_input_id_{};
   };
}

#endif