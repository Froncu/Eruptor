#include "device_builder.hpp"
#include "services/context/context.hpp"
#include "services/locator.hpp"

namespace eru
{
   UniquePointer<VmaAllocator_T> DeviceBuilder::create_allocator(vk::raii::PhysicalDevice const& physical_device,
      vk::raii::Device const& device)
   {
      Context const& context{ Locator::get<Context>() };
      VmaAllocatorCreateInfo const allocator_create_info{
         .flags{},
         .physicalDevice{ *physical_device },
         .device{ *device },
         .preferredLargeHeapBlockSize{},
         .pAllocationCallbacks{},
         .pDeviceMemoryCallbacks{},
         .pHeapSizeLimit{},
         .pVulkanFunctions{},
         .instance{ *context.instance() },
         .vulkanApiVersion{ context.api_version() },
         .pTypeExternalMemoryHandleTypes{},
      };

      VmaAllocator allocator;
      if (vmaCreateAllocator(&allocator_create_info, &allocator) not_eq VK_SUCCESS)
         exception("failed to create Vulkan memory allocator!");

      return { allocator, vmaDestroyAllocator };
   }

   DeviceBuilder& DeviceBuilder::enable_extension(std::string extension_name)
   {
      if (extension_name.empty())
         return *this;

      extension_names_.insert(std::move(extension_name));

      return *this;
   }

   DeviceBuilder& DeviceBuilder::enable_extensions(std::vector<std::string> extension_names)
   {
      for (std::string& extension_name : extension_names)
         enable_extension(std::move(extension_name));

      return *this;
   }

   DeviceBuilder& DeviceBuilder::enable_features10(vk::PhysicalDeviceFeatures const& features)
   {
      // TODO: re-enable this when the templated functions are implemented
      // enable_features(features10_, features);
      features10_ = features;
      return *this;
   }

   DeviceBuilder& DeviceBuilder::enable_features11(vk::PhysicalDeviceVulkan11Features const& features)
   {
      // TODO: re-enable this when the templated functions are implemented
      // enable_features(features11_, features);
      void* const next{ features11_.pNext };
      features11_ = features;
      features11_.pNext = next;

      return *this;
   }

   DeviceBuilder& DeviceBuilder::enable_features12(vk::PhysicalDeviceVulkan12Features const& features)
   {
      // TODO: re-enable this when the templated functions are implemented
      // enable_features(features12_, features);
      void* const next{ features12_.pNext };
      features12_ = features;
      features12_.pNext = next;

      return *this;
   }

   DeviceBuilder& DeviceBuilder::enable_features13(vk::PhysicalDeviceVulkan13Features const& features)
   {
      // TODO: re-enable this when the templated functions are implemented
      // enable_features(features13_, features);
      void* const next{ features13_.pNext };
      features13_ = features;
      features13_.pNext = next;

      return *this;
   }

   DeviceBuilder& DeviceBuilder::add_queues(RequiredQueueInfo const& info, std::uint32_t count)
   {
      if (info.first & vk::QueueFlagBits::eProtected)
         exception("vk::QueueFlagBits::eProtected is not supported!");

      while (count)
      {
         required_queues_.emplace_back(info);
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

   Device DeviceBuilder::build()
   {
      vk::raii::PhysicalDevice physical_device{ pick_physical_device() };
      vk::raii::Device device{ create_device(physical_device) };
      return {
         std::move(physical_device), std::move(device),
         retrieve_queues(device),
         create_command_pools(device),
         create_allocator(physical_device, device)
      };
   }

   vk::raii::PhysicalDevice DeviceBuilder::pick_physical_device()
   {
      std::vector physical_devices{ Locator::get<Context>().instance().enumeratePhysicalDevices() };
      if (physical_devices.empty())
         exception("no physical device with Vulkan support found!");

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
         exception("no physical device with support for requested extensions found!");

      // TODO: re-enable this when the templated functions are implemented
      // std::erase_if(physical_devices,
      //    [this](vk::raii::PhysicalDevice const& physical_device)
      //    {
      //       vk::StructureChain const features{
      //          physical_device.getFeatures2<vk::PhysicalDeviceFeatures2,
      //             vk::PhysicalDeviceVulkan11Features,
      //             vk::PhysicalDeviceVulkan12Features,
      //             vk::PhysicalDeviceVulkan13Features>()
      //       };
      //
      //       if (any_requested_feature_missing(features10_, features.get<vk::PhysicalDeviceFeatures2>().features) or
      //          any_requested_feature_missing(features11_, features.get<vk::PhysicalDeviceVulkan11Features>()) or
      //          any_requested_feature_missing(features12_, features.get<vk::PhysicalDeviceVulkan12Features>()) or
      //          any_requested_feature_missing(features13_, features.get<vk::PhysicalDeviceVulkan13Features>()))
      //          return true;
      //
      //       return false;
      //    });
      // if (physical_devices.empty())
      //    exception("no physical device with support for requested features found!");

      std::unordered_map<VkPhysicalDevice, QueueInfo> queue_infos{};
      std::erase_if(physical_devices,
         [this, &queue_infos](vk::raii::PhysicalDevice const& physical_device)
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

                  if (not available_count or
                     (required_flags & available_flags) not_eq required_flags or
                     (required_surface and not physical_device.getSurfaceSupportKHR(index, required_surface)))
                     continue;

                  --available_count;

                  bool const create_command_pool{
                     required_flags not_eq vk::QueueFlags{} and
                     required_flags not_eq vk::QueueFlagBits::eSparseBinding
                  };

                  queue_infos[*physical_device].emplace_back(index, create_command_pool);

                  found = true;
                  break;
               }

               if (found)
                  continue;

               queue_infos.erase(*physical_device);
               return true;
            }

            return false;
         });
      if (physical_devices.empty())
         exception("no physical device with support for requested queues found!");

      for (vk::raii::PhysicalDevice physical_device : physical_devices)
         if (physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
         {
            queue_info_ = std::move(queue_infos[*physical_device]);
            return physical_device;
         }

      queue_info_ = std::move(queue_infos[*physical_devices.front()]);
      return physical_devices.front();
   }

   vk::raii::Device DeviceBuilder::create_device(vk::raii::PhysicalDevice const& physical_device)
   {
      std::unordered_map<std::uint32_t, std::vector<float>> family_create_counts{};
      for (auto const family_index : std::views::keys(queue_info_))
         family_create_counts[family_index].emplace_back(1.0f);

      std::vector<vk::DeviceQueueCreateInfo> queue_infos{};

      for (auto const& [family_index, priorities] : family_create_counts)
         queue_infos.push_back({
            .queueFamilyIndex{ family_index },
            .queueCount{ static_cast<std::uint32_t>(priorities.size()) },
            .pQueuePriorities{ priorities.data() }
         });

      auto extension_names_view{
         std::views::transform(extension_names_,
            [](std::string const& extension_name)
            {
               return extension_name.c_str();
            })
      };
      std::vector<char const*> extension_names{ extension_names_view.begin(), extension_names_view.end() };

      features_.features = features10_;

      // TODO: for backwards compatibility, enable the same
      // validation layers on each logical device as
      // the ones enabled on the instance from which the
      // logical device is created
      return physical_device.createDevice({
         .pNext{ &features_ },
         .queueCreateInfoCount{ static_cast<std::uint32_t>(queue_infos.size()) },
         .pQueueCreateInfos{ queue_infos.data() },
         .enabledExtensionCount{ static_cast<std::uint32_t>(extension_names.size()) },
         .ppEnabledExtensionNames{ extension_names.data() }
      });
   }

   std::vector<DeviceQueue> DeviceBuilder::retrieve_queues(vk::raii::Device const& device)
   {
      std::unordered_map<std::uint32_t, std::uint32_t> family_create_counts{};

      std::vector<DeviceQueue> queues{};
      for (auto const family_index : std::views::keys(queue_info_))
         queues.push_back(DeviceQueue{
            family_index,
            device.getQueue(family_index, family_create_counts[family_index]++)
         });

      return queues;
   }

   std::unordered_map<std::uint32_t, vk::raii::CommandPool> DeviceBuilder::create_command_pools(vk::raii::Device const& device)
   {
      std::unordered_map<std::uint32_t, vk::raii::CommandPool> command_pools{};
      for (auto const [family_index, create_command_pool] : queue_info_)
         if (create_command_pool)
            command_pools.emplace(family_index, device.createCommandPool({
               .flags{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer },
               .queueFamilyIndex{ family_index }
            }));

      return command_pools;
   }
}