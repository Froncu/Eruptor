#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "events/observer/event_dispatcher.hpp"
#include "events/window_event.hpp"
#include "erupch/erupch.hpp"
#include "reference/reference.hpp"
#include "reference/referenceable.hpp"
#include "services/locator.hpp"
#include "services/system_event_dispatcher/system_event_dispatcher.hpp"
#include "utility/unique_pointer.hpp"
#include "utility/variant_visitor.hpp"

namespace eru
{
   class Window final : public Referenceable
   {
      public:
         explicit Window(std::string_view title = "Application", vk::Extent2D extent = { 640, 480 });

         Window(Window const& other);
         Window(Window&&) = default;

         virtual ~Window() override = default;

         Window& operator=(Window const& other);
         Window& operator=(Window&&) = default;

         void change_title(std::string_view title);
         void change_extent(vk::Extent2D size);
         void change_position(glm::ivec2 position);
         void center();
         void change_fullscreen_mode(bool fullscreen);
         void change_resizability(bool resizable);
         void change_visibility(bool show);

         [[nodiscard]] ID::InternalValue id() const;
         [[nodiscard]] std::string_view title() const;
         [[nodiscard]] vk::Extent2D extent() const;
         [[nodiscard]] glm::ivec2 position() const;
         [[nodiscard]] bool fullscreen() const;
         [[nodiscard]] bool resizable() const;
         [[nodiscard]] bool visible() const;

         [[nodiscard]] vk::raii::SurfaceKHR const& surface() const;

         EventDispatcher<> close_event{};
         EventDispatcher<vk::Extent2D const> resize_event{};

      private:
         Window(std::string_view title, vk::Extent2D size, std::optional<glm::ivec2> const& position, std::uint64_t flags);

         UniquePointer<SDL_Window> native_window_;
         vk::raii::SurfaceKHR surface_;
         EventListener<WindowEvent const> on_window_event_
         {
            VariantVisitor
            {
               [smart_this = Reference<Window>{ this }](WindowCloseEvent const& event)
               {
                  if (smart_this->id() not_eq event.id)
                     return false;

                  smart_this->close_event.notify();
                  return true;
               },

               [smart_this = Reference<Window>{ this }](WindowResizeEvent const& event)
               {
                  if (smart_this->id() not_eq event.id)
                     return false;

                  smart_this->resize_event.notify(event.extent);
                  return true;
               },

               [](auto)
               {
                  return false;
               }
            },
            Locator::get<SystemEventDispatcher>().window_event
         };
   };
}

#endif