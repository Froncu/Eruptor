#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/unique_pointer.hpp"

struct GLFWwindow;

namespace eru
{
   class Window final
   {
      public:
         ERU_API explicit Window(glm::uvec2 extent = { 640, 480 }, std::string_view title = "Window");
         Window(Window const&) = delete;
         Window(Window&&) = delete;

         ERU_API ~Window() = default;

         Window& operator=(Window const&) = delete;
         Window& operator=(Window&&) = delete;

         [[nodiscard]] ERU_API GLFWwindow& native() const;

         ERU_API void change_visibility(bool visible);

         ERU_API void change_extent(glm::uvec2 extent);
         [[nodiscard]] ERU_API glm::uvec2 extent() const;

         ERU_API void change_position(glm::uvec2 position);
         [[nodiscard]] ERU_API glm::uvec2 position() const;

         ERU_API void change_title(std::string_view title);
         [[nodiscard]] ERU_API std::string_view title() const;

      private:
         UniquePointer<GLFWwindow> native_window_;
   };
}

#endif