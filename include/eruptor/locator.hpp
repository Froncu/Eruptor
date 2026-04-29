// Inspired by Matias Devred's locator from his "gooey" engine
// (https://git.allpurposem.at/mat/gooey/src/branch/main/src/lib/services/locator.cppm)

#ifndef LOCATOR_HPP
#define LOCATOR_HPP

#include "eruptor/api.hpp"
#include "eruptor/exception.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/type_index.hpp"
#include "eruptor/unique_pointer.hpp"
#include "eruptor/void_deleter.hpp"

namespace eru
{
   class Locator final
   {
      public:
         class ConstructionKey final
         {
            friend Locator;

            public:
               ConstructionKey(ConstructionKey&&) = default;
               ConstructionKey(ConstructionKey const&) = default;

               ~ConstructionKey() = default;

               auto operator=(ConstructionKey const&) -> ConstructionKey& = delete;
               auto operator=(ConstructionKey&&) -> ConstructionKey& = delete;

            private:
               explicit ConstructionKey() = default;
         };

         template<typename Service, std::derived_from<Service> Provider = Service, typename... Arguments>
            requires std::constructible_from<Provider, ConstructionKey, Arguments...>
         static auto provide(Arguments&&... arguments) -> Service&
         {
            Locator& locator{ instance() };

            UniquePointer<void> new_provider{
               new Provider{ ConstructionKey{}, std::forward<Arguments>(arguments)... },
               void_deleter<Provider>
            };

            auto&& [service_index, did_insert]{ locator.service_indices_.emplace(type_index<Service>(), locator.services_.size()) };
            if (did_insert)
               return *static_cast<Service*>(locator.services_.emplace_back(std::move(new_provider)).get());

            UniquePointer<void>& current_provider{ locator.services_[service_index->second] };

            if constexpr (std::movable<Service>)
               *static_cast<Service*>(new_provider.get()) = std::move(*static_cast<Service*>(current_provider.get()));

            current_provider = std::move(new_provider);
            return *static_cast<Service*>(current_provider.get());
         }

         ERU_API static auto remove_all() -> void;

         template<typename Service>
         [[nodiscard]] static auto get() -> Service&
         {
            Locator& locator{ instance() };

            if (auto const service_index{ locator.service_indices_.find(type_index<Service>()) };
               service_index not_eq locator.service_indices_.end())
               return *static_cast<Service* const>(locator.services_[service_index->second].get());

            throw Exception{
               std::format("attempted to get \"{}\" which hasn't been provided", typeid(Service).name())
            };
         }

         Locator(Locator const&) = delete;
         Locator(Locator&&) = delete;

         auto operator=(Locator const&) -> Locator& = delete;
         auto operator=(Locator&&) -> Locator& = delete;

      private:
         Locator() = default;

         ~Locator();

         // TODO: not a fan of this, but neither do I like the idea of having the user call `Locator::instance()`
         [[nodiscard]] ERU_API static auto instance() -> Locator&;

         std::unordered_map<std::type_index, std::size_t> service_indices_{};
         std::vector<UniquePointer<void>> services_{};
   };
}

#endif