#include "base_reference.hpp"
#include "referenceable.hpp"

namespace eru
{
   BaseReference::BaseReference(Referenceable const* const referencable)
      : referencable_{ const_cast<Referenceable*>(referencable) }
   {
      if (valid())
         referencable_->references_.insert(this);
   }

   BaseReference::BaseReference(Referenceable const& referencable)
      : BaseReference(&referencable)
   {
   }

   BaseReference::BaseReference(BaseReference const& other)
      : BaseReference(other.referencable_)
   {
   }

   BaseReference::BaseReference(BaseReference&& other) noexcept
      : BaseReference(other)
   {
      other.reset();
   }

   BaseReference::~BaseReference()
   {
      if (valid())
         referencable_->references_.erase(this);
   }

   BaseReference& BaseReference::operator=(BaseReference const& other)
   {
      if (*this == other)
         return *this;

      if (valid())
         referencable_->references_.erase(this);

      referencable_ = other.referencable_;

      if (valid())
         referencable_->references_.insert(this);

      return *this;
   }

   BaseReference& BaseReference::operator=(BaseReference&& other) noexcept
   {
      if (this == &other)
         return *this;

      *this = other;

      other.reset();

      return *this;
   }

   bool BaseReference::operator==(BaseReference const& other) const
   {
      return referencable_ == other.referencable_;
   }

   bool BaseReference::operator==(Referenceable const& referencable) const
   {
      return referencable_ == &referencable;
   }

   std::strong_ordering BaseReference::operator<=>(BaseReference const& other) const
   {
      return referencable_ <=> other.referencable_;
   }

   std::strong_ordering BaseReference::operator<=>(Referenceable const& referencable) const
   {
      return referencable_ <=> &referencable;
   }

   void BaseReference::reset()
   {
      if (valid())
      {
         referencable_->references_.erase(this);
         referencable_ = nullptr;
      }
   }

   bool BaseReference::valid() const
   {
      return referencable_;
   }
}