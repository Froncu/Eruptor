#ifndef VARIANT_VISITOR_HPP
#define VARIANT_VISITOR_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   template <typename... Callables>
   class VariantVisitor final
   {
      struct Visitor final : Callables...
      {
         using Callables::operator()...;
      };

      public:
         explicit VariantVisitor(Callables... callables)
            : visitor_{ callables... }
         {
         }

         VariantVisitor(VariantVisitor const&) = default;
         VariantVisitor(VariantVisitor&&) = default;

         ~VariantVisitor() = default;

         VariantVisitor& operator=(VariantVisitor const&) = default;
         VariantVisitor& operator=(VariantVisitor&&) = default;

         template <typename Variant>
         auto operator()(Variant&& variant)
         {
            return std::visit(visitor_, std::forward<Variant>(variant));
         }

      private:
         Visitor visitor_;
   };
}

#endif