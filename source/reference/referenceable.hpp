#ifndef REFERENCABLE_HPP
#define REFERENCABLE_HPP

#include "base_reference.hpp"

namespace eru
{
   class Referenceable
   {
      friend BaseReference;

      public:
         virtual ~Referenceable();

      protected:
         Referenceable() = default;
         Referenceable(Referenceable const&);
         Referenceable(Referenceable&& other) noexcept;

         Referenceable& operator=(Referenceable const&);
         Referenceable& operator=(Referenceable&& other) noexcept;

      private:
         std::unordered_set<BaseReference*> references_{};
   };
}

#endif