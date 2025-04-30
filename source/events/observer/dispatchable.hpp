#ifndef DISPATCHABLE_HPP
#define DISPATCHABLE_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   template <typename Type>
   concept Dispatchable =
      not std::is_reference_v<Type>;
}

#endif