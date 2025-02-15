#include <SDL3/SDL_main.h>

#include "application/application.hpp"

int main(int const, char** const)
{
   SDL_Init(SDL_INIT_EVENTS);

   try
   {
      eru::application{}.run();
   }
   catch (std::exception const& exception)
   {
      std::cout << std::format("exception caught: {}\n", exception.what());
   }

   SDL_Quit();

   return 0;
}