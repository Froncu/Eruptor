#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "eruptor/eruptor.hpp"

namespace eru
{
   [[nodiscard]] Application* create_application(std::span<char const* const> arguments);
}

SDL_AppResult SDL_AppInit(void** const appstate, int const argc, char** const argv) try
{
   *appstate = eru::create_application({ argv, static_cast<std::size_t>(argc) });
   return SDL_APP_CONTINUE;
}
catch (std::exception const& exception)
{
   eru::Logger{}.error(std::format("failed to initialize the application! ({})", exception.what()));
   return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppIterate(void* const appstate) try
{
   return
      static_cast<eru::Application*>(appstate)->tick()
         ? SDL_APP_CONTINUE
         : SDL_APP_SUCCESS;
}
catch (std::exception const& exception)
{
   eru::Locator::get<eru::Logger>().error(std::format("failed to iterate the application! ({})", exception.what()));
   return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void* const appstate, SDL_Event* const event) try
{
   return
      static_cast<eru::Application*>(appstate)->process_event(*event)
         ? SDL_APP_CONTINUE
         : SDL_APP_SUCCESS;
}
catch (std::exception const& exception)
{
   eru::Locator::get<eru::Logger>().error(std::format("failed to process an application event! ({})", exception.what()));
   return SDL_APP_FAILURE;
}

void SDL_AppQuit(void* const appstate, SDL_AppResult const)
{
   delete static_cast<eru::Application*>(appstate);
}