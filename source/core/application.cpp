#include <SDL3/SDL.h>

#include "eruptor/application.hpp"
#include "eruptor/locator.hpp"
#include "eruptor/logger.hpp"
#include "eruptor/runtime_assert.hpp"
#include "eruptor/window.hpp"

namespace eru
{
   SDL_InitFlags constexpr INITIALIZATION_FLAGS{
      SDL_INIT_EVENTS |
      SDL_INIT_VIDEO
   };

   Application::~Application()
   {
      Locator::remove_all();

      SDL_QuitSubSystem(INITIALIZATION_FLAGS);
      SDL_Quit();
   }

   Application::Application()
   {
      bool const succeeded{ SDL_InitSubSystem(INITIALIZATION_FLAGS) };
      runtime_assert(succeeded, std::format("failed to initialize SDL subsystems! ({})", SDL_GetError()));

      Locator::provide<Application>(*this);
      Locator::provide<Logger>();
      Locator::provide<Window>();
   }
}