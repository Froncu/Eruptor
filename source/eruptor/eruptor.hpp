#ifndef ERUPTOR_HPP
#define ERUPTOR_HPP

#include "renderer/renderer.hpp"
#include "services/input_manager/input_manager.hpp"
#include "window/window.hpp"

namespace eru
{
   class Eruptor
   {
      public:
         Eruptor();
         Eruptor(Eruptor const&) = delete;
         Eruptor(Eruptor&&) = delete;

         ~Eruptor() = default;

         Eruptor& operator=(Eruptor const&) = delete;
         Eruptor& operator=(Eruptor&&) = delete;

         void run();

      private:
         Window window_{ "Eruptor", { 1280, 720 } };
         Renderer renderer_{ window_ };
         EventListener<> on_window_close_{
            [this]
            {
               is_running_ = false;
               return true;
            },
            window_.close_event
         };

         UserInput const& user_input_{ Locator::get<InputManager>().user_input(0) };
         EventListener<float const> on_mouse_lock_{
            [this](float const value)
            {
               bool const lock_mouse{ value > 0.0f };

               if (lock_mouse)
                  user_input_.bind_action("rotate", VectorAction{
                     .positive_x_inputs{ MouseAxis::WEST, GamepadAxis::RIGHT_STICK_WEST },
                     .negative_x_inputs{ MouseAxis::EAST, GamepadAxis::RIGHT_STICK_EAST },
                     .positive_y_inputs{ MouseAxis::NORTH, GamepadAxis::RIGHT_STICK_NORTH },
                     .negative_y_inputs{ MouseAxis::SOUTH, GamepadAxis::RIGHT_STICK_SOUTH }
                  });
               else
                  user_input_.bind_action("rotate", VectorAction{
                     .positive_x_inputs{ GamepadAxis::RIGHT_STICK_WEST },
                     .negative_x_inputs{ GamepadAxis::RIGHT_STICK_EAST },
                     .positive_y_inputs{ GamepadAxis::RIGHT_STICK_NORTH },
                     .negative_y_inputs{ GamepadAxis::RIGHT_STICK_SOUTH }
                  });

               window_.change_lock_mouse(lock_mouse);
               return true;
            },
            user_input_.value_action_event("lock_mouse")
         };

         float movement_speed_{ 2.0f };
         float rotation_speed_{ 64.0f };
         bool is_running_{ true };
   };
}

#endif