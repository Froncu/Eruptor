#ifndef LOCATOR_HPP
#define LOCATOR_HPP

#include "erupch/erupch.hpp"
#include "utility/type_index.hpp"
#include "utility/unique_pointer.hpp"
#include "utility/void_deleter.hpp"

namespace eru
{
   class Locator final
   {
      public:
         template <std::default_initializable Service, std::derived_from<Service> Provider, typename... Arguments>
            requires std::constructible_from<Provider, Arguments...>
         static void set(Arguments&&... arguments)
         {
            if constexpr (std::movable<Service> and std::movable<Provider>)
            {
               UniquePointer<void>& current_provider{ internal_get<Service>() };
               Provider new_provider{ std::forward<Arguments>(arguments)... };
               static_cast<Service&>(new_provider) = std::move(*static_cast<Service*>(current_provider.get()));
               current_provider = UniquePointer<void>{ new Provider{ std::move(new_provider) }, void_deleter<Provider> };
            }
            else
               services_[type_index<Service>()] =
                  UniquePointer<void>{ new Provider{ std::forward<Arguments>(arguments)... }, void_deleter<Provider> };
         }

         template <std::default_initializable Service>
         static void reset()
         {
            services_[type_index<Service>()] =
               UniquePointer<void>{ new Service{}, void_deleter<Service> };
         }

         template <std::default_initializable Service>
         [[nodiscard]] static Service& get()
         {
            return *static_cast<Service*>(internal_get<Service>().get());
         }

         Locator() = delete;
         Locator(Locator const&) = delete;
         Locator(Locator&&) = delete;

         ~Locator() = delete;

         Locator& operator=(Locator const&) = delete;
         Locator& operator=(Locator&&) = delete;

      private:
         template <std::default_initializable Service>
         [[nodiscard]] static UniquePointer<void>& internal_get()
         {
            auto service{ services_.find(type_index<Service>()) };
            if (service == services_.end())
               service = services_.emplace(type_index<Service>(),
                  UniquePointer<void>{ new Service{}, void_deleter<Service> }).first;

            return service->second;
         }

         static std::unordered_map<std::type_index, UniquePointer<void>> services_;
   };
}

#endif