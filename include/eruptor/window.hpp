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
         [[nodiscard]] ERU_API static auto required_instance_extension_names() -> std::span<char const* const>;
         [[nodiscard]] ERU_API static auto presentation_support(vk::raii::Instance const& instance, vk::raii::PhysicalDevice const& physical_device,
            std::uint32_t queue_family_index) -> bool;

         ERU_API explicit Window(glm::uvec2 extent = { 640, 480 }, std::string_view title = "Window");
         Window(Window const&) = delete;
         Window(Window&&) = delete;

         ERU_API ~Window() = default;

         auto operator=(Window const&) -> Window& = delete;
         auto operator=(Window&&) -> Window& = delete;

         [[nodiscard]] ERU_API auto native() const -> GLFWwindow&;

         ERU_API auto change_visibility(bool visible) -> void;

         ERU_API auto change_extent(glm::uvec2 extent) -> void;
         [[nodiscard]] ERU_API auto extent() const -> glm::uvec2;

         ERU_API auto change_position(glm::uvec2 position) -> void;
         [[nodiscard]] ERU_API auto position() const -> glm::uvec2;

         ERU_API auto change_title(std::string_view title) -> void;
         [[nodiscard]] ERU_API auto title() const -> std::string_view;

      private:
         UniquePointer<GLFWwindow> native_window_;
   };
}

#endif