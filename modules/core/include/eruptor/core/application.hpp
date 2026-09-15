#pragma once

#include <eruptor/core/api.hpp>
#include <eruptor/core/locator.hpp>

namespace eru
{
   class Application
   {
      public:
         Application(Application const&) = delete;
         Application(Application&&) noexcept = delete;

         virtual ~Application() = default;

         auto operator=(Application const&) -> Application& = delete;
         auto operator=(Application&&) -> Application& = delete;

         [[nodiscard]] virtual auto tick() -> bool = 0;

      protected:
         ERU_API explicit Application(PassKey<Locator>);
   };
}