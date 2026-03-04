#ifndef UNIQUE_POINTER_HPP
#define UNIQUE_POINTER_HPP

#include "eruptor/pch.hpp"

namespace eru
{
   template <typename Resource>
   using UniquePointer = std::unique_ptr<Resource, std::function<void(Resource*)>>;
}

#endif