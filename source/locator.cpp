#include "eruptor/locator.hpp"

namespace eru
{
   void Locator::remove_all()
   {
      viewed_services_.clear();

      while (not owned_services_.empty())
         owned_services_.pop_back();

      owned_service_indices_.clear();
   }

   decltype(Locator::owned_service_indices_) Locator::owned_service_indices_{};
   decltype(Locator::owned_services_) Locator::owned_services_{};
   decltype(Locator::viewed_services_) Locator::viewed_services_{};
}