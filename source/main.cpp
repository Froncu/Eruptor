#include <SDL3/SDL_main.h>

#include "builders/context_builder.hpp"
#include "erupch/erupch.hpp"
#include "eruptor/eruptor.hpp"
#include "services/locator.hpp"
#include "utility/constants.hpp"

int main(int const, char** const)
{
   SDL_InitFlags constexpr initialization_flags{
      SDL_INIT_EVENTS |
      SDL_INIT_VIDEO |
      SDL_INIT_GAMEPAD
   };

   SDL_InitSubSystem(initialization_flags);

   eru::Locator::set<eru::Context, eru::Context>(
      eru::ContextBuilder{}
      .change_api_version(vk::ApiVersion13)
      .enable_validation_layer(eru::constants::DEBUG ? "VK_LAYER_KHRONOS_validation" : "")
      .enable_extension(eru::constants::DEBUG ? vk::EXTDebugUtilsExtensionName : "")
      .build());

   eru::Eruptor{}.run();

   eru::Locator::reset();

   SDL_QuitSubSystem(initialization_flags);

   return 0;
}