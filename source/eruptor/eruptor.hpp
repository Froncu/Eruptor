#ifndef ERUPTOR_HPP
#define ERUPTOR_HPP

#include "renderer/renderer.hpp"
#include "window/window.hpp"

namespace eru
{
   class Eruptor
   {
      public:
         Eruptor() = default;
         Eruptor(Eruptor const&) = delete;
         Eruptor(Eruptor&&) = delete;

         ~Eruptor() = default;

         Eruptor& operator=(Eruptor const&) = delete;
         Eruptor& operator=(Eruptor&&) = delete;

         void run();

      private:
         Window window_{ "Eruptor" };
         Renderer renderer_{ window_ };
         EventListener<> on_window_close_{
            [this]
            {
               is_running_ = false;
               return true;
            },
            window_.close_event
         };
         bool is_running_{ true };
   };
}

#endif