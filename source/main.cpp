#include <SDL3/SDL_main.h>

#include "eruptor/eruptor.hpp"

int main(int const, char** const)
{
   SDL_InitFlags constexpr initialization_flags{
      SDL_INIT_EVENTS |
      SDL_INIT_VIDEO |
      SDL_INIT_GAMEPAD
   };

   SDL_InitSubSystem(initialization_flags);

   try
   {
      eru::Eruptor{}.run();
   }
   catch (std::exception const& exception)
   {
      std::cout << std::format("exception caught: {}\n", exception.what());
   }

   SDL_QuitSubSystem(initialization_flags);

   return 0;
}