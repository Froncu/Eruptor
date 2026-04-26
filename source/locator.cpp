#include "eruptor/locator.hpp"

namespace eru
{
   void Locator::remove_all()
   {
      Locator& locator{ instance() };

      locator.viewed_services_.clear();

      while (not locator.owned_services_.empty())
         locator.owned_services_.pop_back();

      locator.owned_service_indices_.clear();
   }

   Locator& Locator::instance()
   {
      static Locator locator{};
      return locator;
   }
}