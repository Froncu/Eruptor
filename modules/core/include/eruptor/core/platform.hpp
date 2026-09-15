#pragma once

#include <eruptor/core/api.hpp>
#include <eruptor/core/pass_key.hpp>

namespace eru
{
   class Locator;

   class Platform final
   {
      public:
         ERU_API explicit Platform(PassKey<Locator>);
         Platform(Platform const&) = delete;
         Platform(Platform&&) = delete;

         ERU_API ~Platform();

         auto operator=(Platform const&) -> Platform& = delete;
         auto operator=(Platform&&) -> Platform& = delete;

         ERU_API auto poll() const -> void;
   };
}