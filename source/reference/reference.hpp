#ifndef REFERENCE_HPP
#define REFERENCE_HPP

#include "base_reference.hpp"

namespace eru
{
   // TODO: there should be a constraint/static assert on Type to be derived from Referenceable.
   // This is not implemented because such a constraint limits the Reference when used as a
   // data member; class A cannot hold a data member of type Reference<A> even though A
   // derives from Referenceable. This is because A is not complete when the constraint
   // is checked.

   template <typename Type>
   concept NonReference =
      not std::derived_from<Type, BaseReference> and
      not std::is_reference_v<Type>;

   template <typename From, typename To>
   concept ConstantPromotable =
      not std::is_const_v<From> or
      std::is_const_v<To>;

   template <typename From, typename To>
   concept ReferenceConstructable =
      NonReference<From> and
      ConstantPromotable<From, To> and
      std::derived_from<From, To>;

   template <typename... Ts>
   concept Polymorphic =
      (std::is_polymorphic_v<Ts> and ...);

   template <typename T1, typename T2>
   concept EitherDerived =
      std::derived_from<T1, T2> or
      std::derived_from<T2, T1>;

   template <NonReference T>
   class Reference final : public BaseReference
   {
      public:
         Reference() = default;

         template <ReferenceConstructable<T> Other>
         explicit Reference(Other* const referencable)
            : BaseReference(referencable)
         {
         }

         template <ReferenceConstructable<T> Other>
         explicit Reference(Other& referencable)
            : BaseReference(referencable)
         {
         }

         explicit Reference(T* const referencable)
            : BaseReference(referencable)
         {
         }

         explicit Reference(T& referencable)
            : BaseReference(referencable)
         {
         }

         template <ReferenceConstructable<T> Other>
         explicit Reference(Reference<Other> const* const referencable)
            : BaseReference(referencable)
         {
         }

         template <ReferenceConstructable<T> Other>
         explicit Reference(Reference<Other> const& referencable)
            : BaseReference(referencable)
         {
         }

         Reference(Reference const& other)
            : BaseReference(other)
         {
         }

         Reference(Reference&& other) noexcept
            : BaseReference(std::move(other))
         {
         }

         virtual ~Reference() override = default;

         Reference& operator=(Reference const& other)
         {
            BaseReference::operator=(other);
            return *this;
         }

         Reference& operator=(Reference&& other) noexcept
         {
            BaseReference::operator=(std::move(other));
            return *this;
         }

         [[nodiscard]] T* operator->() const
         {
            return &get();
         }

         [[nodiscard]] T& operator*() const
         {
            return get();
         }

         template <NonReference Other>
            requires Polymorphic<T, Other> and ConstantPromotable<T, Other>
         [[nodiscard]] Reference<Other> cast_dynamically() const
         {
            return dynamic_cast<Other* const>(referencable_);
         }

         template <NonReference Other>
            requires EitherDerived<T, Other> and ConstantPromotable<T, Other>
         [[nodiscard]] Reference<Other> cast_statically() const
         {
            return static_cast<Other* const>(referencable_);
         }

         [[nodiscard]] T& get() const
         {
            return *static_cast<T* const>(referencable_);
         }
   };
}

#endif