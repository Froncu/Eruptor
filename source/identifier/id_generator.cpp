#include "id.hpp"
#include "id_generator.hpp"
#include "utility/exception.hpp"

namespace eru
{
   IDGenerator::IDGenerator(IDGenerator&& other) noexcept
      : Referenceable(std::move(other))
      , highest_taken_id_{ other.highest_taken_id_ }
      , free_ids_{ std::move(other.free_ids_) }
   {
      other.highest_taken_id_ = ID::INVALID_ID;
   }

   IDGenerator& IDGenerator::operator=(IDGenerator&& other) noexcept
   {
      if (this == &other)
         return *this;

      Referenceable::operator=(std::move(other));
      highest_taken_id_ = other.highest_taken_id_;
      free_ids_ = std::move(other.free_ids_);

      other.highest_taken_id_ = ID::INVALID_ID;

      return *this;
   }

   ID IDGenerator::generate()
   {
      return ID{ *this };
   }

   IDGenerator::InternalValue IDGenerator::internal_generate()
   {
      InternalValue id;

      if (free_ids_.empty())
      {
         if (highest_taken_id_ == ID::MAX_ID)
            exception("the ID generator has run out of IDs");

         id = ++highest_taken_id_;
      }
      else
      {
         auto const lowest_free_id{ --free_ids_.end() };
         id = *lowest_free_id;
         free_ids_.erase(lowest_free_id);
      }

      return id;
   }
}