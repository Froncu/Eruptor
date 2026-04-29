#include "eruptor/locator.hpp"

namespace eru
{
   auto Locator::remove_all() -> void
   {
      Locator& locator{ instance() };

      while (not locator.services_.empty())
         locator.services_.pop_back();

      locator.service_indices_.clear();
   }

   Locator::~Locator()
   {
      remove_all();
   }

   auto Locator::instance() -> Locator&
   {
      static Locator locator{};
      return locator;
   }
}