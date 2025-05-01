#include "user_input.hpp"

namespace eru
{
   float UserInput::deadzoned_strength(float const strength, float const deadzone)
   {
      return strength < deadzone ? 0.0f : (strength - deadzone) / (1.0f - deadzone);
   }

   float UserInput::highest_strength(std::unordered_set<Input> const& inputs,
      std::unordered_map<Input, float>& input_strengths)
   {
      auto highest_strength{ std::numeric_limits<float>::lowest() };
      for (Input const input : inputs)
      {
         if (float const strength{ input_strengths[input] };
            strength > highest_strength)
            highest_strength = strength;
      }

      return highest_strength;
   }

   UserInput::UserInput(int const id)
      : id_{ id }
   {
   }

   bool UserInput::operator==(int const id) const
   {
      return id_ == id;
   }

   bool UserInput::operator==(UserInput const& other) const
   {
      return *this == other.id_;
   }

   EventDispatcher<float const>& UserInput::bind_action(std::string const& action_name, ValueAction action) const
   {
      InternalValueAction& internal_action{ value_actions_[action_name] };
      internal_action.action = std::move(action);
      return internal_action.value_changed_event;
   }

   EventDispatcher<float const>& UserInput::bind_action(std::string const& action_name, AxisAction action) const
   {
      InternalAxisAction& internal_action{ axis_actions_[action_name] };
      internal_action.action = std::move(action);
      return internal_action.value_changed_event;
   }

   EventDispatcher<glm::vec2 const>& UserInput::bind_action(std::string const& action_name, VectorAction action) const
   {
      InternalVectorAction& internal_action{ vector_actions_[action_name] };
      internal_action.action = std::move(action);
      return internal_action.value_changed_event;
   }

   EventDispatcher<float const>& UserInput::value_action(std::string const& action_name) const
   {
      return value_actions_[action_name].value_changed_event;
   }

   EventDispatcher<float const>& UserInput::axis_action(std::string const& action_name) const
   {
      return axis_actions_[action_name].value_changed_event;
   }

   EventDispatcher<glm::vec2 const>& UserInput::vector_action(std::string const& action_name) const
   {
      return vector_actions_[action_name].value_changed_event;
   }

   float UserInput::value_action_strength(std::string const& action_name) const
   {
      return value_actions_[action_name].value;
   }

   float UserInput::axis_action_strength(std::string const& action_name) const
   {
      return axis_actions_[action_name].value;
   }

   glm::vec2 UserInput::vector_action_strength(std::string const& action_name) const
   {
      return vector_actions_[action_name].value;
   }

   int UserInput::id() const
   {
      return id_;
   }

   void UserInput::calculate_action_values_if(std::function<bool(std::unordered_set<Input> const&)> const& predicate) const
   {
      for (InternalValueAction& internal_action : std::views::values(value_actions_))
         if (predicate(internal_action.action.inputs))
         {
            auto& [action, value_changed_event, value] = internal_action;
            auto const& [inputs, deadzone] = action;

            float const old_value{ value };

            value = deadzoned_strength(highest_strength(inputs, input_strengths_), deadzone);

            if (old_value not_eq value)
               value_changed_event.notify(value);
         }

      for (InternalAxisAction& internal_action : std::views::values(axis_actions_))
         if (predicate(internal_action.action.positive_inputs) or
            predicate(internal_action.action.negative_inputs))
         {
            auto& [action, value_changed_event, value] = internal_action;
            auto const& [positive_inputs, negative_inputs, deadzone] = action;

            float const old_value{ value };

            value =
               deadzoned_strength(highest_strength(positive_inputs, input_strengths_), deadzone) -
               deadzoned_strength(highest_strength(negative_inputs, input_strengths_), deadzone);

            if (old_value not_eq value)
               value_changed_event.notify(value);
         }

      for (InternalVectorAction& internal_action : std::views::values(vector_actions_))
         if (predicate(internal_action.action.positive_x_inputs) or
            predicate(internal_action.action.negative_x_inputs) or
            predicate(internal_action.action.positive_y_inputs) or
            predicate(internal_action.action.negative_y_inputs))
         {
            auto& [action, value_changed_event, value] = internal_action;
            auto const& [positive_x_inputs, negative_x_inputs, positive_y_inputs, negative_y_inputs, deadzone] = action;

            glm::vec2 const old_value{ value };

            value = {
               highest_strength(positive_x_inputs, input_strengths_) - highest_strength(negative_x_inputs, input_strengths_),
               highest_strength(positive_y_inputs, input_strengths_) - highest_strength(negative_y_inputs, input_strengths_)
            };

            if (float const magnitude{ glm::length(value) }; magnitude > 1.0)
               value /= magnitude;
            else
               value *= deadzoned_strength(magnitude, deadzone);

            if (old_value not_eq value)
               value_changed_event.notify(value);
         }
   }

   void UserInput::reset_input_strength_if(std::function<bool(Input)> const& predicate) const
   {
      std::unordered_set<Input> reset_inputs{};
      for (auto& [input, strength] : input_strengths_)
         if (strength not_eq decltype(strength){} and predicate(input))
         {
            strength = {};
            reset_inputs.insert(input);
         }

      if (reset_inputs.empty())
         return;

      calculate_action_values_if(
         [&reset_inputs](std::unordered_set<Input> const& inputs)
         {
            return std::ranges::any_of(inputs,
               [&reset_inputs](Input const input)
               {
                  return reset_inputs.contains(input);
               });
         });
   }

   void UserInput::move_input_strength_if(UserInput const& user_input, std::function<bool(Input)> const& predicate) const
   {
      std::unordered_set<Input> moved_inputs{};
      for (auto& [input, other_strength] : user_input.input_strengths_)
         if (float& strength{ input_strengths_[input] }; strength not_eq other_strength and predicate(input))
         {
            strength = other_strength;
            other_strength = {};
            moved_inputs.insert(input);
         }

      if (moved_inputs.empty())
         return;

      auto const action_predicate{
         [&moved_inputs](std::unordered_set<Input> const& inputs)
         {
            return std::ranges::any_of(inputs,
               [&moved_inputs](Input const input)
               {
                  return moved_inputs.contains(input);
               });
         }
      };

      user_input.calculate_action_values_if(action_predicate);
      calculate_action_values_if(action_predicate);
   }

   void UserInput::swap_input_strengths_if(UserInput const& user_input, std::function<bool(Input)> const& predicate) const
   {
      std::unordered_map<Input, float>& other_input_strengths{ user_input.input_strengths_ };

      std::unordered_set<Input> swapped_inputs{};
      auto const swap{
         [&predicate, &swapped_inputs](float& strength_a, float& strength_b, Input const input)
         {
            if (strength_a == strength_b or not predicate(input))
               return;

            std::swap(strength_a, strength_b);
            swapped_inputs.insert(input);
         }
      };

      for (Input const input : std::views::keys(input_strengths_))
         swap(input_strengths_[input], other_input_strengths[input], input);

      for (Input const input : std::views::keys(other_input_strengths))
         if (not swapped_inputs.contains(input))
            swap(input_strengths_[input], other_input_strengths[input], input);

      if (swapped_inputs.empty())
         return;

      auto const action_predicate{
         [&swapped_inputs](std::unordered_set<Input> const& inputs)
         {
            return std::ranges::any_of(inputs,
               [&swapped_inputs](Input const input)
               {
                  return swapped_inputs.contains(input);
               });
         }
      };

      user_input.calculate_action_values_if(action_predicate);
      calculate_action_values_if(action_predicate);
   }

   void UserInput::change_input_strength(Input input, float const strength) const
   {
      float& current_strength{ input_strengths_[input] };
      if (current_strength == strength)
         return;

      current_strength = strength;
      calculate_action_values_if(
         [input](std::unordered_set<Input> const& inputs)
         {
            return inputs.contains(input);
         });
   }
}