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
         UserInput const& user_input_{ Locator::get<InputManager>().user_input(0) };
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
         float movement_speed_{ 2.0f };
         float rotation_speed_{ 64.0f };
         bool is_running_{ true };
   };
}

#endif