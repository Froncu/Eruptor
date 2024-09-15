#include "erupch.hpp"

#include "application.hpp"

namespace eru
{
   application::application()
   {
   }

   application::~application()
   {
      instance_.destroy();
   }

   void application::run() const
   {
      while (not glfwWindowShouldClose(window_.get()))
         glfwPollEvents();
   }

   vk::Instance application::create_instance() const
   {
      std::uint32_t extension_count;
      char const* const* const extensions{ glfwGetRequiredInstanceExtensions(&extension_count) };
      std::vector<char const*> const extension_names{ extensions, extensions + extension_count };

#if defined NDEBUG
      std::vector<char const*> const valdiation_layer_names{};
#else
      std::vector const valdiation_layer_names{ "VK_LAYER_KHRONOS_validation" };
#endif

      vk::InstanceCreateInfo const instance_info{
         {},
         {},
         valdiation_layer_names,
         extension_names
      };

      return vk::createInstance(instance_info);
   }
}