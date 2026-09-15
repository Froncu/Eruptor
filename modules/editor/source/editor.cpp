#include <eruptor/entry/entry.hpp>

#include "editor.hpp"

namespace eru
{
   auto provide_application(std::span<char const* const> const) -> void
   {
      Locator::provide<Application, mgm::Editor>();
   }
}

namespace mgm
{
   Editor::Editor(eru::PassKey<eru::Locator> const construction_key)
      : Application{ construction_key }
   {
   }

   auto Editor::tick() -> bool
   {
      platform_.poll();
      return true;
   }
}