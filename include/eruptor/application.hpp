#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "eruptor/api.hpp"
#include "eruptor/locator.hpp"

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
         ERU_API explicit Application(Locator::ConstructionKey);
   };
}

#endif