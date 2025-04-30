#include "base_reference.hpp"
#include "referenceable.hpp"

namespace eru
{
   Referenceable::Referenceable(Referenceable const&)
   {
   }

   Referenceable::Referenceable(Referenceable&& other) noexcept
      : references_{ std::move(other.references_) }
   {
      for (BaseReference* const reference : references_)
         reference->referencable_ = this;
   }

   Referenceable::~Referenceable()
   {
      for (BaseReference* const reference : references_)
         reference->referencable_ = nullptr;
   }

   Referenceable& Referenceable::operator=(Referenceable const&)
   {
      return *this;
   }

   Referenceable& Referenceable::operator=(Referenceable&& other) noexcept
   {
      for (BaseReference* const reference : other.references_)
         reference->referencable_ = this;

      references_.merge(other.references_);

      return *this;
   }
}