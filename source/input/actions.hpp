#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include "input.hpp"

namespace eru
{
   struct ValueAction final
   {
      std::unordered_set<Input> inputs{};
      double deadzone{ 0.25 };
   };

   struct AxisAction final
   {
      std::unordered_set<Input> positive_inputs{};
      std::unordered_set<Input> negative_inputs{};
      double deadzone{ 0.25 };
   };

   struct VectorAction final
   {
      std::unordered_set<Input> positive_x_inputs{};
      std::unordered_set<Input> negative_x_inputs{};
      std::unordered_set<Input> positive_y_inputs{};
      std::unordered_set<Input> negative_y_inputs{};
      double deadzone{ 0.25 };
   };
}

#endif