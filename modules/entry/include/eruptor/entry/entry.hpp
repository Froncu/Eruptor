#pragma once

#include <span>

namespace eru
{
   auto provide_application(std::span<char const* const> arguments) -> void;
}