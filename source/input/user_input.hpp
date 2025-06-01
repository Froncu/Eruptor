#ifndef USER_INPUT_HPP
#define USER_INPUT_HPP

#include "actions.hpp"
#include "erupch/erupch.hpp"
#include "events/observer/event_dispatcher.hpp"
#include "reference/referenceable.hpp"

namespace eru
{
   class UserInput final : public Referenceable
   {
      friend class InputManager;

      struct InternalValueAction final
      {
         ValueAction action{};
         EventDispatcher<float const> value_changed_event{};
         float value{};
      };

      struct InternalAxisAction final
      {
         AxisAction action{};
         EventDispatcher<float const> value_changed_event{};
         float value{};
      };

      struct InternalVectorAction final
      {
         VectorAction action{};
         EventDispatcher<glm::vec2 const> value_changed_event{};
         glm::vec2 value{};
      };

      public:
         static int constexpr INVALID_USER_ID{ -1 };

         UserInput(UserInput const&) = delete;
         UserInput(UserInput&&) = default;

         virtual ~UserInput() override = default;

         UserInput& operator=(UserInput const&) = delete;
         UserInput& operator=(UserInput&&) = default;
         [[nodiscard]] bool operator==(int id) const;
         [[nodiscard]] bool operator==(UserInput const& other) const;

         EventDispatcher<float const>& bind_action(std::string const& action_name, ValueAction action) const;
         EventDispatcher<float const>& bind_action(std::string const& action_name, AxisAction action) const;
         EventDispatcher<glm::vec2 const>& bind_action(std::string const& action_name, VectorAction action) const;

         [[nodiscard]] EventDispatcher<float const>& value_action_event(std::string const& action_name) const;
         [[nodiscard]] EventDispatcher<float const>& axis_action_event(std::string const& action_name) const;
         [[nodiscard]] EventDispatcher<glm::vec2 const>& vector_action_event(std::string const& action_name) const;

         [[nodiscard]] float value_action_strength(std::string const& action_name) const;
         [[nodiscard]] float axis_action_strength(std::string const& action_name) const;
         [[nodiscard]] glm::vec2 vector_action_strength(std::string const& action_name) const;
         [[nodiscard]] int id() const;

      private:
         [[nodiscard]] static float deadzoned_strength(float strength, float deadzone);
         [[nodiscard]] static float highest_strength(std::unordered_set<Input> const& inputs,
            std::unordered_map<Input, float>& input_strengths);

         explicit UserInput(int id);

         void calculate_action_values_if(std::function<bool(std::unordered_set<Input> const&)> const& predicate) const;
         void reset_input_strength_if(std::function<bool(Input)> const& predicate) const;
         void move_input_strength_if(UserInput const& user_input, std::function<bool(Input)> const& predicate) const;
         void swap_input_strengths_if(UserInput const& user_input, std::function<bool(Input)> const& predicate) const;
         void change_input_strength(Input input, float strength) const;

         mutable std::unordered_map<Input, float> input_strengths_{};
         mutable std::unordered_map<std::string, InternalValueAction> value_actions_{};
         mutable std::unordered_map<std::string, InternalAxisAction> axis_actions_{};
         mutable std::unordered_map<std::string, InternalVectorAction> vector_actions_{};

         int id_;
   };
}

template <>
struct std::hash<eru::UserInput>
{
   using is_transparent = void;

   [[nodiscard]] std::size_t operator()(eru::UserInput const& user_input) const noexcept
   {
      return user_input.id();
   }

   [[nodiscard]] std::size_t operator()(int const id) const noexcept
   {
      return id;
   }
};

template <>
struct std::equal_to<eru::UserInput>
{
   using is_transparent = void;

   [[nodiscard]] bool operator()(eru::UserInput const& user_input_a, eru::UserInput const& user_input_b) const noexcept
   {
      return user_input_a == user_input_b;
   }

   [[nodiscard]] bool operator()(int const id, eru::UserInput const& user_input) const noexcept
   {
      return user_input == id;
   }
};

#endif