#ifndef BASE_REFERENCE_HPP
#define BASE_REFERENCE_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   class Referenceable;

   class BaseReference
   {
      friend Referenceable;

      public:
         virtual ~BaseReference();

         [[nodiscard]] bool operator==(Referenceable const& referencable) const;
         [[nodiscard]] bool operator==(BaseReference const& other) const;
         [[nodiscard]] std::strong_ordering operator<=>(BaseReference const& other) const;
         [[nodiscard]] std::strong_ordering operator<=>(Referenceable const& referencable) const;

         void reset();
         bool valid() const;

      protected:
         BaseReference() = default;
         explicit BaseReference(Referenceable const* referencable);
         explicit BaseReference(Referenceable const& referencable);
         BaseReference(BaseReference const& other);
         BaseReference(BaseReference&& other) noexcept;

         BaseReference& operator=(BaseReference&& other) noexcept;
         BaseReference& operator=(BaseReference const& other);

         Referenceable* referencable_{};
   };
}

#endif