// Inspired by Matias Devred's locator from his "gooey" engine
// (https://git.allpurposem.at/mat/gooey/src/branch/main/src/lib/services/locator.cppm)

#ifndef LOCATOR_HPP
#define LOCATOR_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/type_index.hpp"
#include "eruptor/unique_pointer.hpp"
#include "eruptor/void_deleter.hpp"

namespace eru
{
   class Locator final
   {
      public:
         template <typename Service, std::derived_from<Service> Provider = Service, typename... Arguments>
            requires std::constructible_from<Provider, Arguments...>
         static Service& provide(Arguments&&... arguments)
         {
            UniquePointer<void> new_provider{ new Provider{ std::forward<Arguments>(arguments)... }, void_deleter<Provider> };

            auto&& [service_index, did_insert]{ service_indices_.emplace(type_index<Service>(), services_.size()) };
            if (did_insert)
               return *static_cast<Service*>(services_.emplace_back(std::move(new_provider)).get());

            UniquePointer<void>& current_provider{ services_[service_index->second] };

            if constexpr (std::movable<Service>)
               *static_cast<Service*>(new_provider.get()) = std::move(*static_cast<Service*>(current_provider.get()));

            current_provider = std::move(new_provider);
            return *static_cast<Service*>(current_provider.get());
         }

         static void remove_providers()
         {
            while (not services_.empty())
               services_.pop_back();

            service_indices_.clear();
         }

         template <typename Service>
         [[nodiscard]] static Service& get()
         {
            auto const service_index{ service_indices_.find(type_index<Service>()) };
            if (service_index == service_indices_.end())
               throw std::runtime_error{
                  std::format("attempted to get \"{}\" which hasn't been provided", typeid(Service).name())
               };
            return *static_cast<Service* const>(services_[service_index->second].get());
         }

         Locator() = delete;
         Locator(Locator const&) = delete;
         Locator(Locator&&) = delete;

         ~Locator() = delete;

         Locator& operator=(Locator const&) = delete;
         Locator& operator=(Locator&&) = delete;

      private:
         ERU_API static std::unordered_map<std::type_index, std::size_t> service_indices_;
         ERU_API static std::vector<UniquePointer<void>> services_;
   };
}

#endif