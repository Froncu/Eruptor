#include "eruptor/locator.hpp"
#include "eruptor/window.hpp"

#include "core/dependencies.hpp"

namespace eru
{
   auto Window::required_instance_extension_names() -> std::vector<std::string_view>
   {
      std::uint32_t required_instance_extensions_count;
      char const* const* const required_instance_extensions{
         glfwGetRequiredInstanceExtensions(&required_instance_extensions_count)
      };

      return { required_instance_extensions, required_instance_extensions + required_instance_extensions_count };
   }

   auto Window::presentation_support(vk::raii::Instance const& instance, vk::raii::PhysicalDevice const& physical_device, std::uint32_t const queue_family_index) -> bool
   {
      return glfwGetPhysicalDevicePresentationSupport(*instance, *physical_device, queue_family_index);
   }

   Window::Window(glm::uvec2 const extent, std::string_view const title, SwapChain::Description const& swap_chain_description)
      : native_window_{ glfwCreateWindow(extent.x, extent.y, title.data(), nullptr, nullptr), glfwDestroyWindow }
      , surface_{
         [this] [[nodiscard]] -> vk::raii::SurfaceKHR
         {
            static vk::raii::Instance const& INSTANCE{ Locator::get<Context>().instance };

            VkSurfaceKHR surface;
            glfwCreateWindowSurface(*INSTANCE, native_window_.get(), nullptr, &surface);
            return { INSTANCE, surface };
         }()
      }
      , swap_chain_{ {}, surface_, swap_chain_description }
   {
   }

   auto Window::change_visibility(bool const visible) -> void
   {
      visible
         ? glfwShowWindow(native_window_.get())
         : glfwHideWindow(native_window_.get());
   }

   auto Window::change_extent(glm::uvec2 const extent) -> void
   {
      glfwSetWindowSize(native_window_.get(), extent.x, extent.y);
   }

   auto Window::extent(bool const in_physical_pixels) const -> glm::uvec2
   {
      glm::ivec2 extent;
      in_physical_pixels ?
         glfwGetFramebufferSize(native_window_.get(), &extent.x, &extent.y) :
         glfwGetWindowSize(native_window_.get(), &extent.x, &extent.y);

      return extent;
   }

   auto Window::change_position(glm::uvec2 const position) -> void
   {
      glfwSetWindowPos(native_window_.get(), position.x, position.y);
   }

   auto Window::position() const -> glm::uvec2
   {
      glm::ivec2 position;
      glfwGetWindowPos(native_window_.get(), &position.x, &position.y);
      return position;
   }

   auto Window::change_title(std::string_view const title) -> void
   {
      glfwSetWindowTitle(native_window_.get(), title.data());
   }

   auto Window::title() const -> std::string_view
   {
      return glfwGetWindowTitle(native_window_.get());
   }

   auto Window::swap_chain() const -> SwapChain const&
   {
      return swap_chain_;
   }
}