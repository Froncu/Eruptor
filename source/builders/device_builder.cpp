#include "device_builder.hpp"

namespace eru
{
   DeviceBuilder& DeviceBuilder::enable_extension(std::string extension_name)
   {
      extension_names_.insert(std::move(extension_name));
      return *this;
   }

   DeviceBuilder& DeviceBuilder::enable_extensions(std::initializer_list<std::string> const extension_names)
   {
      extension_names_.insert(std::make_move_iterator(extension_names.begin()), std::make_move_iterator(extension_names.end()));
      return *this;
   }

   DeviceBuilder& DeviceBuilder::enable_features(vk::PhysicalDeviceFeatures const& features)
   {
      auto const target_features{ reinterpret_cast<vk::Bool32*>(&features_) };
      auto const source_features{ reinterpret_cast<vk::Bool32 const*>(&features) };

      for (std::size_t index{}; index < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++index)
         if (not target_features[index] and source_features[index])
            target_features[index] = true;

      return *this;
   }

   DeviceBuilder& DeviceBuilder::add_queues(RequiredQueueInfo const& info, std::uint32_t count)
   {
      while (count)
      {
         required_queues_.push_back(info);
         --count;
      }
      return *this;
   }

   DeviceBuilder& DeviceBuilder::add_queues(vk::QueueFlags flags, std::uint32_t const count)
   {
      add_queues({ flags, {} }, count);
      return *this;
   }

   DeviceBuilder& DeviceBuilder::add_queues(vk::raii::SurfaceKHR surface, std::uint32_t const count)
   {
      add_queues({ {}, surface }, count);
      return *this;
   }

   Device DeviceBuilder::build(Context const& context)
   {
      queue_infos_.clear();

      vk::raii::PhysicalDevice physical_device{ pick_physical_device(context) };
      vk::raii::Device device{ create_device(physical_device) };
      return {
         std::move(physical_device), std::move(device),
         retrieve_queues(physical_device, device),
         create_command_pools(physical_device, device)
      };
   }

   vk::raii::PhysicalDevice DeviceBuilder::pick_physical_device(Context const& context)
   {
      std::vector physical_devices{ context.instance().enumeratePhysicalDevices() };
      if (physical_devices.empty())
         throw std::runtime_error("no physical device with Vulkan support found!");

      std::erase_if(physical_devices,
         [this](vk::raii::PhysicalDevice const& physical_device)
         {
            auto extracted_available_extension_names{
               std::views::transform(physical_device.enumerateDeviceExtensionProperties(),
                  [](vk::ExtensionProperties const& extension_property) -> std::string_view
                  {
                     return extension_property.extensionName;
                  })
            };

            std::set<std::string_view> const available_extension_names{
               extracted_available_extension_names.begin(), extracted_available_extension_names.end()
            };

            return not std::ranges::includes(available_extension_names, extension_names_);
         });
      if (physical_devices.empty())
         throw std::runtime_error("no physical device with support for requested extensions found!");

      auto const required_features{ reinterpret_cast<vk::Bool32 const*>(&features_) };
      std::erase_if(physical_devices,
         [required_features](vk::raii::PhysicalDevice const& physical_device)
         {
            vk::PhysicalDeviceFeatures const features{ physical_device.getFeatures() };
            auto const available_features{ reinterpret_cast<vk::Bool32 const*>(&features) };

            for (std::size_t index{}; index < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++index)
               if (required_features[index] and not available_features[index])
                  return true;

            return false;
         });
      if (physical_devices.empty())
         throw std::runtime_error("no physical device with support for requested features found!");

      std::erase_if(physical_devices,
         [this](vk::raii::PhysicalDevice const& physical_device)
         {
            auto extracted_available_queues{
               std::views::transform(physical_device.getQueueFamilyProperties(),
                  [](vk::QueueFamilyProperties const& queue_family_properties)
                  {
                     return std::pair{
                        queue_family_properties.queueFlags, queue_family_properties.queueCount
                     };
                  })
            };

            std::vector<std::pair<vk::QueueFlags, std::uint32_t>> available_queues{
               extracted_available_queues.begin(), extracted_available_queues.end()
            };

            for (auto const [required_flags, required_surface] : required_queues_)
            {
               bool found{};
               for (std::uint32_t index{}; index < available_queues.size(); ++index)
               {
                  auto& [available_flags, available_count]{ available_queues[index] };

                  if ((required_flags & available_flags) not_eq required_flags or
                     (required_surface and not physical_device.getSurfaceSupportKHR(index, required_surface)) or
                     not available_count)
                     continue;

                  --available_count;

                  auto& [family_create_counts, retrieval_infos]{ queue_infos_[*physical_device] };
                  ++family_create_counts[index];
                  retrieval_infos.emplace_back(index);

                  found = true;
                  break;
               }

               if (found)
                  continue;

               queue_infos_.erase(*physical_device);
               return true;
            }

            return false;
         });
      if (physical_devices.empty())
         throw std::runtime_error("no physical device with support for requested queues found!");

      for (vk::raii::PhysicalDevice physical_device : physical_devices)
         if (physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            return physical_device;

      return physical_devices.front();
   }

   vk::raii::Device DeviceBuilder::create_device(vk::raii::PhysicalDevice const& physical_device)
   {
      std::vector<std::vector<float>> queue_priorities{};

      std::vector<vk::DeviceQueueCreateInfo> queue_infos{};
      for (auto const [family_index, count] : queue_infos_[*physical_device].family_create_counts)
         queue_infos.push_back({
            .queueFamilyIndex{ family_index },
            .queueCount{ count },
            .pQueuePriorities{ queue_priorities.emplace_back(std::vector<float>(count)).data() }
         });

      auto extension_names_view{
         std::views::transform(extension_names_,
            [](std::string const& extension_name) -> char const*
            {
               return extension_name.c_str();
            })
      };
      std::vector<char const*> extension_names{ extension_names_view.begin(), extension_names_view.end() };

      // TODO: for backwards compatibility, enable the same
      // validation layers on each logical device as
      // the ones enabled on the instance from which the
      // logical device is created
      return physical_device.createDevice({
         .queueCreateInfoCount{ static_cast<std::uint32_t>(queue_infos.size()) },
         .pQueueCreateInfos{ queue_infos.data() },
         .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
         .ppEnabledExtensionNames{ extension_names.data() },
         .pEnabledFeatures{ &features_ }
      });
   }

   std::vector<DeviceQueue> DeviceBuilder::retrieve_queues(vk::raii::PhysicalDevice const& physical_device,
      vk::raii::Device const& device)
   {
      auto& [family_create_counts, retrieval_infos]{ queue_infos_[*physical_device] };

      for (std::uint32_t& family_create_count : std::views::values(family_create_counts))
         family_create_count = 0;

      std::vector<DeviceQueue> queues{};
      for (std::uint32_t family_index : retrieval_infos)
         queues.push_back(DeviceQueue{
            family_index,
            device.getQueue(family_index, family_create_counts[family_index]++)
         });

      return queues;
   }

   std::unordered_map<std::uint32_t, vk::raii::CommandPool> DeviceBuilder::create_command_pools(
      vk::raii::PhysicalDevice const& physical_device, vk::raii::Device const& device)
   {
      std::unordered_map<std::uint32_t, vk::raii::CommandPool> command_pools{};
      for (std::uint32_t const family_index : std::views::keys(queue_infos_[*physical_device].family_create_counts))
         command_pools.emplace(family_index, device.createCommandPool({
            .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
            .queueFamilyIndex{ family_index }
         }));

      return command_pools;
   }
}