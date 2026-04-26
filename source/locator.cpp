#include "eruptor/locator.hpp"

namespace eru
{
   auto Locator::remove_all() -> void
   {
      Locator& locator{ instance() };

      locator.viewed_services_.clear();

      while (not locator.owned_services_.empty())
         locator.owned_services_.pop_back();

      locator.owned_service_indices_.clear();
   }

   auto Locator::instance() -> Locator&
   {
      static Locator locator{};
      return locator;
   }
}