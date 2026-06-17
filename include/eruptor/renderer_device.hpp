#ifndef RENDERER_DEVICE_HPP
#define RENDERER_DEVICE_HPP

#include "eruptor/pch.hpp"
#include "eruptor/runtime_assert.hpp"
#include "eruptor/unique_parameter_pack.hpp"

namespace eru
{
   class RendererDevice final
   {
      public:
         template<typename... ChainElements>
            requires IS_UNIQUE<ChainElements...>
         RendererDevice(vk::raii::Instance const& instance, std::initializer_list<char const* const> const device_extension_names, ChainElements&&... device_features)
            : physical_device_{
               [&instance]
               {
                  vk::ResultValue const physical_devices{ instance.enumeratePhysicalDevices() };
                  RUNTIME_ASSERT(physical_devices.has_value(),
                     std::format("failed to query available physical devices! ({})", to_string(physical_devices.result)));

                  if (physical_devices->empty())
                     throw std::runtime_error{ "no compatible physical devices found!" };

                  auto const discrete_physical_device{
                     std::ranges::find_if(*physical_devices,
                        [](vk::raii::PhysicalDevice const& found_physical_device)
                        {
                           return found_physical_device.getProperties2().properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
                        })
                  };

                  if (discrete_physical_device not_eq std::ranges::end(*physical_devices))
                     return std::move(*discrete_physical_device);

                  return std::move(physical_devices->front());
               }()
            }
            , device_{
               [this, &instance, &device_extension_names, &device_features...]
               {
                  vk::StructureChain const requested_features{ device_features... };

                  std::ranges::enumerate_view const queue_family_properties{ physical_device_.getQueueFamilyProperties2() };
                  auto const queue_family{
                     std::ranges::find_if(queue_family_properties,
                        [this, &instance](auto&& pair)
                        {
                           auto&& [index, properties]{ pair };
                           return Window::presentation_support(instance, physical_device_, static_cast<std::uint32_t>(index))
                              and static_cast<bool>(properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
                        })
                  };
                  RUNTIME_ASSERT(queue_family not_eq std::ranges::end(queue_family_properties),
                     "no suitable queue family found!");

                  auto constexpr queue_priority{ 0.5f };
                  std::array const device_queue_create_info{
                     std::to_array<vk::DeviceQueueCreateInfo>({
                        {
                           .queueFamilyIndex{ static_cast<std::uint32_t>(queue_family.index()) },
                           .queueCount{ 1 },
                           .pQueuePriorities{ &queue_priority }
                        }
                     })
                  };

                  vk::ResultValue device{
                     physical_device_.createDevice({
                        .pNext{ requested_features.get() },
                        .queueCreateInfoCount{ static_cast<std::uint32_t>(std::ranges::size(device_queue_create_info)) },
                        .pQueueCreateInfos{ std::ranges::data(device_queue_create_info) },
                        .enabledExtensionCount{ static_cast<std::uint32_t>(std::ranges::size(device_extension_names)) },
                        .ppEnabledExtensionNames{ std::ranges::data(device_extension_names) }
                     })
                  };
                  RUNTIME_ASSERT(device.has_value(),
                     std::format("failed to create device! ({})", to_string(device.result)));

                  return std::move(*device);
               }()
            }
         {
         }

         RendererDevice(RendererDevice const&) = delete;
         RendererDevice(RendererDevice&&) = delete;

         ~RendererDevice() = default;

         auto operator=(RendererDevice const&) -> RendererDevice& = delete;
         auto operator=(RendererDevice&&) -> RendererDevice& = delete;

      private:
         vk::raii::PhysicalDevice const physical_device_;
         vk::raii::Device const device_;
   };
}

#endif