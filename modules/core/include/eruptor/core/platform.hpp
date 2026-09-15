#pragma once

#include <eruptor/core/pass_key.hpp>

namespace eru
{
   class Locator;

   class Platform final
   {
      public:
         ERU_CORE_API explicit Platform(PassKey<Locator>);
         Platform(Platform const&) = delete;
         Platform(Platform&&) = delete;

         ERU_CORE_API ~Platform();

         auto operator=(Platform const&) -> Platform& = delete;
         auto operator=(Platform&&) -> Platform& = delete;

         ERU_CORE_API auto poll() const -> void;
   };
}