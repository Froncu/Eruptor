#pragma once

#include <functional>

namespace eru
{
   template<typename Resource>
   using UniquePointer = std::unique_ptr<Resource, std::function<void(Resource*)>>;
}