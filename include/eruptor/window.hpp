#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/unique_pointer.hpp"

namespace eru
{
   class Window final
   {
      public:
         ERU_API Window();
         Window(Window const&) = delete;
         Window(Window&&) = delete;

         ERU_API ~Window() = default;

         Window& operator=(Window const&) = delete;
         Window& operator=(Window&&) = delete;

         ERU_API void change_visibility(bool visible);

         ERU_API void change_extent(glm::uvec2 extent);
         ERU_API [[nodiscard]] glm::uvec2 extent() const;

         ERU_API void change_position(glm::uvec2 position);
         ERU_API [[nodiscard]] glm::uvec2 position() const;

         ERU_API void change_title(std::string_view title);
         ERU_API [[nodiscard]] std::string_view title() const;

      private:
         UniquePointer<struct GLFWwindow> native_window_;
   };
}

#endif