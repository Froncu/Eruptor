#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"

namespace eru
{
   class Application
   {
      public:
         Application(Application const&) = delete;
         Application(Application&&) noexcept = delete;

         ERU_API virtual ~Application();

         Application& operator=(Application const&) = delete;
         Application& operator=(Application&&) = delete;

         [[nodiscard]] virtual bool tick() = 0;
         [[nodiscard]] virtual bool process_event(SDL_Event& event) = 0;

      protected:
         ERU_API Application();
   };
}

#endif