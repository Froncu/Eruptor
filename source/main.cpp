#include <SDL3/SDL_main.h>

#include "eruptor/eruptor.hpp"

int main(int const, char** const)
{
   SDL_InitSubSystem(SDL_INIT_VIDEO);

   try
   {
      eru::Eruptor{}.run();
   }
   catch (std::exception const& exception)
   {
      std::cout << std::format("exception caught: {}\n", exception.what());
   }

   SDL_QuitSubSystem(SDL_INIT_VIDEO);

   return 0;
}