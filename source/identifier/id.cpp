#include "id.hpp"
#include "utility/exception.hpp"

namespace eru
{
   ID::ID(IDGenerator& generator)
      : generator_{ generator }
      , value_{ generator_->internal_generate() }
   {
   }

   ID::ID(ID const& other)
      : generator_{ other.generator_ }
      , value_{ generator_.valid() ? generator_->internal_generate() : INVALID_ID }
   {
   }

   ID::ID(ID&& other) noexcept
      : generator_{ std::move(other.generator_) }
      , value_{ other.value_ }
   {
      other.value_ = INVALID_ID;
   }

   ID::~ID()
   {
      free_value();
   }

   ID& ID::operator=(ID const&)
   {
      return *this;
   }

   ID& ID::operator=(ID&& other)
   {
      if (this == &other)
         return *this;

      if (generator_ not_eq other.generator_)
         exception("cannot move assign ID with different generators");

      free_value();
      value_ = other.value_;

      other.generator_.reset();
      other.value_ = INVALID_ID;

      return *this;
   }

   ID::operator InternalValue() const
   {
      return value_;
   }

   bool ID::operator==(ID const& other) const
   {
      if (not generator_.valid() or not other.generator_.valid())
         return false;

      return
         generator_ == other.generator_ and
         value_ == other.value_;
   }

   std::partial_ordering ID::operator<=>(ID const& other) const
   {
      if (not generator_.valid() or not other.generator_.valid())
         return std::partial_ordering::unordered;

      if (generator_ == other.generator_)
         return value_ <=> other.value_;

      return generator_ <=> other.generator_;
   }

   Reference<IDGenerator> ID::generator() const
   {
      return generator_;
   }

   void ID::free_value() const
   {
      if (value_ == INVALID_ID or not generator_.valid())
         return;

      InternalValue& highest_taken_id{ generator_->highest_taken_id_ };
      auto& free_ids{ generator_->free_ids_ };

      if (value_ == highest_taken_id)
      {
         --highest_taken_id;

         auto begin{ free_ids.begin() };
         while (begin not_eq free_ids.end())
         {
            auto const highest_free_id{ begin++ };
            if (highest_taken_id not_eq *highest_free_id)
               break;

            free_ids.erase(highest_free_id);
            --highest_taken_id;
         }
      }
      else
         free_ids.insert(value_);
   }
}