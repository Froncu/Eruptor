#ifndef ID_HPP
#define ID_HPP

#include "erupch/erupch.hpp"
#include "id_generator.hpp"
#include "reference/reference.hpp"

namespace eru
{
   class ID final
   {
      friend IDGenerator;

      public:
         using InternalValue = IDGenerator::InternalValue;
         static InternalValue constexpr INVALID_ID{ IDGenerator::INVALID_ID };
         static InternalValue constexpr MAX_ID{ IDGenerator::MAX_ID };

         ID(ID const& other);
         ID(ID&& other) noexcept;

         ~ID();

         ID& operator=(ID const& other);
         ID& operator=(ID&& other) noexcept(false);
         [[nodiscard]] explicit operator InternalValue() const;
         [[nodiscard]] bool operator==(ID const& other) const;
         [[nodiscard]] std::partial_ordering operator<=>(ID const& other) const;

         [[nodiscard]] Reference<IDGenerator> generator() const;

      private:
         explicit ID(IDGenerator& generator);

         void free_value() const;

         Reference<IDGenerator> generator_;
         InternalValue value_;
   };
}

template <>
struct std::formatter<eru::ID>
{
   static constexpr auto parse(std::format_parse_context const& context)
   {
      return context.begin();
   }

   static auto format(eru::ID const& id, std::format_context& context)
   {
      if (static_cast<eru::ID::InternalValue>(id) == eru::ID::INVALID_ID)
         return std::format_to(context.out(), "{}", "INVALID");

      return std::format_to(context.out(), "{}", static_cast<eru::ID::InternalValue>(id));
   }
};

#endif