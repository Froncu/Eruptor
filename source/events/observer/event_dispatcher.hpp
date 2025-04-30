#ifndef EVENT_DISPATCHER_HPP
#define EVENT_DISPATCHER_HPP

#include "dispatchable.hpp"
#include "event_listener.hpp"
#include "erupch/erupch.hpp"

namespace eru
{
   template <Dispatchable...>
   class EventListener;

   template <Dispatchable... Payload>
   class EventDispatcher final
   {
      using EventListenerType = EventListener<Payload...>;
      friend EventListenerType;

      public:
         template <std::same_as<EventListenerType>... EventListenerTypes>
         explicit EventDispatcher(EventListenerTypes&... event_listeners)
         {
            (add_listener(event_listeners), ...);
         }

         EventDispatcher(EventDispatcher const&)
         {
         }

         EventDispatcher(EventDispatcher&& other) noexcept
            : listeners_{ std::move(other.listeners_) }
         {
            for (auto const listener : listeners_)
            {
               listener->dispatchers_.erase(&other);
               listener->dispatchers_.insert(this);
            }
         }

         ~EventDispatcher()
         {
            for (auto const listener : listeners_)
               listener->dispatchers_.erase(this);
         }

         EventDispatcher& operator=(EventDispatcher const&)
         {
            return *this;
         }

         EventDispatcher& operator=(EventDispatcher&& other) noexcept
         {
            if (this == &other)
               return *this;

            for (auto const listener : listeners_)
               listener->dispatchers_.erase(this);

            listeners_ = std::move(other.listeners_);

            for (auto const listener : listeners_)
            {
               listener->dispatchers_.erase(&other);
               listener->dispatchers_.insert(this);
            }

            return *this;
         }

         bool notify(Payload&... payload)
         {
            bool did_any_listener_handle{};

            for (auto const listener : listeners_)
               if (listener->on_notify_(payload...))
                  did_any_listener_handle = true;

            return did_any_listener_handle;
         }

         void add_listener(EventListenerType& event_listener)
         {
            listeners_.insert(&event_listener);
            event_listener.dispatchers_.insert(this);
         }

         void remove_listener(EventListenerType& event_listener)
         {
            listeners_.erase(&event_listener);
            event_listener.dispatchers_.erase(this);
         }

         void remove_all_listeners()
         {
            for (auto const listener : listeners_)
               listener->dispatchers_.erase(this);

            listeners_.clear();
         }

      private:
         std::unordered_set<EventListenerType*> listeners_{};
   };
}

#endif