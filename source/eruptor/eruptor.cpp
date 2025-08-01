#include "eruptor.hpp"

namespace eru
{
   Eruptor::Eruptor()
   {
      window_.change_resizability(true);
      Locator::get<InputManager>().change_mouse_sensitivity(0.2f);

      user_input_.bind_action("change_movement_speed", AxisAction{
         .positive_inputs{ Key::UP, GamepadButton::WEST },
         .negative_inputs{ Key::DOWN, GamepadButton::SOUTH }
      });

      user_input_.bind_action("move", VectorAction{
         .positive_x_inputs{ Key::D, GamepadAxis::LEFT_STICK_EAST },
         .negative_x_inputs{ Key::A, GamepadAxis::LEFT_STICK_WEST },
         .positive_y_inputs{ Key::W, GamepadAxis::LEFT_STICK_NORTH },
         .negative_y_inputs{ Key::S, GamepadAxis::LEFT_STICK_SOUTH }
      });

      user_input_.bind_action("up_down", AxisAction{
         .positive_inputs{ Key::LEFT_SHIFT, GamepadAxis::RIGHT_TRIGGER },
         .negative_inputs{ Key::LEFT_CTRL, GamepadAxis::LEFT_TRIGGER }
      });

      user_input_.bind_action("rotate", VectorAction{
         .positive_x_inputs{ GamepadAxis::RIGHT_STICK_EAST },
         .negative_x_inputs{ GamepadAxis::RIGHT_STICK_WEST },
         .positive_y_inputs{ GamepadAxis::RIGHT_STICK_NORTH },
         .negative_y_inputs{ GamepadAxis::RIGHT_STICK_SOUTH }
      });

      user_input_.bind_action("lock_mouse", ValueAction{
         .inputs{ MouseButton::LEFT, MouseButton::RIGHT }
      });
   }

   void Eruptor::run()
   {
      auto last_time{ std::chrono::high_resolution_clock::now() };
      while (is_running_)
      {
         Locator::get<SystemEventDispatcher>().poll_events();

         auto const current_time{ std::chrono::high_resolution_clock::now() };
         float const delta_seconds{ std::chrono::duration<float>(current_time - last_time).count() };
         last_time = current_time;

         glm::vec2 move_direction{ user_input_.vector_action_strength("move") };
         move_direction *= movement_speed_ * delta_seconds;
         renderer_.camera.translate_in_direction({ move_direction.x, 0.0f, move_direction.y });

         float up_down{ user_input_.axis_action_strength("up_down") };
         up_down *= movement_speed_ * delta_seconds;
         renderer_.camera.translate({ 0.0f, up_down, 0.0f });

         // TODO: mouse rotation sort of snaps to a line
         glm::vec2 rotation{ user_input_.vector_action_strength("rotate") };
         rotation *= rotation_speed_ * delta_seconds;
         renderer_.camera.rotate(rotation.x, rotation.y);

         renderer_.render(delta_seconds);
      }
   }
}