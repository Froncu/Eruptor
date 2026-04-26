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

            auto&& [service_index, did_insert]{ locator.owned_service_indices_.emplace(type_index<Service>(), locator.owned_services_.size()) };
            if (did_insert)
               return *static_cast<Service*>(locator.owned_services_.emplace_back(std::move(new_provider)).get());

            UniquePointer<void>& current_provider{ locator.owned_services_[service_index->second] };

            if constexpr (std::movable<Service>)
               *static_cast<Service*>(new_provider.get()) = std::move(*static_cast<Service*>(current_provider.get()));

            current_provider = std::move(new_provider);
            return *static_cast<Service*>(current_provider.get());
         }

         template<typename Service, std::derived_from<Service> Provider = Service>
         static auto provide(Provider& provider) -> Service&
         {
            Locator& locator{ instance() };

            locator.viewed_services_[type_index<Service>()] = &provider;
            return provider;
         }

         ERU_API static auto remove_all() -> void;

         template<typename Service>
         [[nodiscard]] static auto get() -> Service&
         {
            Locator& locator{ instance() };

            if (auto const viewed_service{ locator.viewed_services_.find(type_index<Service>()) };
               viewed_service not_eq locator.viewed_services_.end())
               return *static_cast<Service*>(viewed_service->second);

            if (auto const owned_service_index{ locator.owned_service_indices_.find(type_index<Service>()) };
               owned_service_index not_eq locator.owned_service_indices_.end())
               return *static_cast<Service* const>(locator.owned_services_[owned_service_index->second].get());

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

         ~Locator() = default;

         // TODO: not a fan of this, but neither do I like the idea of having the user call `Locator::instance()`
         [[nodiscard]] ERU_API static auto instance() -> Locator&;

         std::unordered_map<std::type_index, std::size_t> owned_service_indices_{};
         std::vector<UniquePointer<void>> owned_services_{};
         std::unordered_map<std::type_index, void*> viewed_services_{};
   };
}

#endif