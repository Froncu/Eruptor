#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "locator.hpp"

namespace eru
{
   class Platform final
   {
      public:
         ERU_API Platform(Locator::ConstructionKey);
         Platform(Platform const&) = delete;
         Platform(Platform&&) = delete;

         ERU_API ~Platform();

         auto operator=(Platform const&) -> Platform& = delete;
         auto operator=(Platform&&) -> Platform& = delete;

         auto poll() const -> void;
   };
}

#endif