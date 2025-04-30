#ifndef EVENT_LISTENER_HPP
#define EVENT_LISTENER_HPP

#include "dispatchable.hpp"
#include "event_dispatcher.hpp"
#include "erupch/erupch.hpp"
#include "utility/exception.hpp"

namespace eru
{
   template <Dispatchable...>
   class EventDispatcher;

   template <Dispatchable... Payload>
   class EventListener final
   {
      using EventDispatcherType = EventDispatcher<Payload...>;
      using CallbackType = std::function<bool(Payload&...)>;
      friend EventDispatcherType;

      public:
         template <std::same_as<EventDispatcherType>... EventDispatcherTypes>
         explicit EventListener(CallbackType on_notify = {}, EventDispatcherTypes&... event_dispatchers)
            : on_notify_{ std::move(on_notify) }
         {
            if (not on_notify_)
               exception("the on_notify callback cannot be a null!");

            (subscribe(event_dispatchers), ...);
         }

         EventListener(EventListener const& other)
            : on_notify_{ other.on_notify_ }
            , dispatchers_{ other.dispatchers_ }
         {
            for (auto const dispatcher : dispatchers_)
               dispatcher->listeners_.insert(this);
         }

         EventListener(EventListener&& other) noexcept
            : on_notify_{ std::move(other.on_notify_) }
            , dispatchers_{ std::move(other.dispatchers_) }
         {
            for (auto const dispatcher : dispatchers_)
            {
               dispatcher->listeners_.erase(&other);
               dispatcher->listeners_.insert(this);
            }
         }

         ~EventListener()
         {
            for (auto const dispatcher : dispatchers_)
               dispatcher->listeners_.erase(this);
         }

         EventListener& operator=(EventListener const& other)
         {
            if (this == &other)
               return *this;

            on_notify_ = other.on_notify_;

            for (auto const dispatcher : dispatchers_)
               dispatcher->listeners_.erase(this);

            dispatchers_ = other.dispatchers_;

            for (auto const dispatcher : dispatchers_)
               dispatcher->listeners_.insert(this);

            return *this;
         }

         EventListener& operator=(EventListener&& other) noexcept
         {
            if (this == &other)
               return *this;

            on_notify_ = std::move(other.on_notify_);

            for (auto const dispatcher : dispatchers_)
               dispatcher->listeners_.erase(this);

            dispatchers_ = std::move(other.dispatchers_);

            for (auto const dispatcher : dispatchers_)
            {
               dispatcher->listeners_.erase(&other);
               dispatcher->listeners_.insert(this);
            }

            return *this;
         }

         EventListener& operator=(CallbackType on_notify)
         {
            if (not on_notify)
               exception("the on_notify callback cannot be a nullptr!");

            on_notify_ = std::move(on_notify);
            return *this;
         }

         bool operator()(Payload&... payload)
         {
            return on_notify_(payload...);
         }

         void subscribe(EventDispatcherType& event_dispatcher)
         {
            dispatchers_.insert(&event_dispatcher);
            event_dispatcher.listeners_.insert(this);
         }

         void unsubscribe(EventDispatcherType& event_dispatcher)
         {
            dispatchers_.erase(&event_dispatcher);
            event_dispatcher.listeners_.erase(this);
         }

         void unsubscribe_all()
         {
            for (auto const dispatcher : dispatchers_)
               dispatcher->listeners_.erase(this);

            dispatchers_.clear();
         }

      private:
         CallbackType on_notify_;
         std::unordered_set<EventDispatcherType*> dispatchers_{};
   };
}

#endif