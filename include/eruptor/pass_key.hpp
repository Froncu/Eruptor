#ifndef PASS_KEY_HPP
#define PASS_KEY_HPP

namespace eru
{
   template<typename Parent>
   class PassKey
   {
      friend Parent;

      public:
         PassKey(PassKey&&) = default;
         PassKey(PassKey const&) = default;

         ~PassKey() = default;

         auto operator=(PassKey const&) -> PassKey& = delete;
         auto operator=(PassKey&&) -> PassKey& = delete;

      private:
         explicit PassKey() = default;
   };
}

#endif