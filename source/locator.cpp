#include "eruptor/locator.hpp"

namespace eru
{
   decltype(Locator::service_indices_) Locator::service_indices_{};
   decltype(Locator::services_) Locator::services_{};
}