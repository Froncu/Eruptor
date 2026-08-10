#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "eruptor/api.hpp"
#include "eruptor/pch.hpp"
#include "eruptor/swap_chain.hpp"
#include "eruptor/unique_pointer.hpp"

namespace eru
{
   class Window final
   {
      using NativeHandle = struct GLFWwindow;

      public:
         [[nodiscard]] ERU_API static auto required_instance_extension_names() -> std::vector<std::string_view>;
         [[nodiscard]] ERU_API static auto presentation_support(vk::raii::Instance const& instance, vk::raii::PhysicalDevice const& physical_device, std::uint32_t queue_family_index) -> bool;

         ERU_API explicit Window(glm::uvec2 extent = { 640, 480 }, std::string_view title = "Window", SwapChain::Description const& swap_chain_description = {});
         Window(Window const&) = delete;
         Window(Window&&) = delete;

         ERU_API ~Window() = default;

         auto operator=(Window const&) -> Window& = delete;
         auto operator=(Window&&) -> Window& = delete;

         ERU_API auto change_visibility(bool visible) -> void;

         ERU_API auto change_extent(glm::uvec2 extent) -> void;
         [[nodiscard]] ERU_API auto extent(bool in_physical_pixels = false) const -> glm::uvec2;

         ERU_API auto change_position(glm::uvec2 position) -> void;
         [[nodiscard]] ERU_API auto position() const -> glm::uvec2;

         ERU_API auto change_title(std::string_view title) -> void;
         [[nodiscard]] ERU_API auto title() const -> std::string_view;

         [[nodiscard]] ERU_API auto swap_chain() const -> SwapChain const&;

      private:
         UniquePointer<NativeHandle> native_window_;
         vk::raii::SurfaceKHR surface_;
         SwapChain swap_chain_;
   };
}

#endif