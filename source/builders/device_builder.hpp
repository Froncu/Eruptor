#ifndef DEVICE_BUILDER_HPP
#define DEVICE_BUILDER_HPP

#include "erupch/erupch.hpp"
#include "renderer/device.hpp"
#include "renderer/device_queue.hpp"

namespace eru
{
   template <typename Type>
   concept PhysicaDeviceFeatureStruct =
      std::is_same_v<Type, vk::PhysicalDeviceFeatures> or
      std::is_same_v<Type, vk::PhysicalDeviceVulkan11Features> or
      std::is_same_v<Type, vk::PhysicalDeviceVulkan12Features> or
      std::is_same_v<Type, vk::PhysicalDeviceVulkan13Features>;

   class DeviceBuilder final
   {
      using QueueInfo = std::vector<std::pair<std::uint32_t, bool>>;

      public:
         using RequiredQueueInfo = std::pair<vk::QueueFlags, vk::SurfaceKHR>;

         DeviceBuilder() = default;
         DeviceBuilder(DeviceBuilder const&) = delete;
         DeviceBuilder(DeviceBuilder&&) = delete;

         ~DeviceBuilder() = default;

         DeviceBuilder& operator=(DeviceBuilder const&) = delete;
         DeviceBuilder& operator=(DeviceBuilder&&) = delete;

         DeviceBuilder& enable_extension(std::string extension_name);
         DeviceBuilder& enable_extensions(std::vector<std::string> extension_names);
         DeviceBuilder& enable_features10(vk::PhysicalDeviceFeatures const& features);
         DeviceBuilder& enable_features11(vk::PhysicalDeviceVulkan11Features const& features);
         DeviceBuilder& enable_features12(vk::PhysicalDeviceVulkan12Features const& features);
         DeviceBuilder& enable_features13(vk::PhysicalDeviceVulkan13Features const& features);
         DeviceBuilder& add_queues(RequiredQueueInfo const& info, std::uint32_t count = 1);
         DeviceBuilder& add_queues(vk::QueueFlags flags, std::uint32_t count = 1);
         DeviceBuilder& add_queues(vk::raii::SurfaceKHR surface, std::uint32_t count = 1);

         [[nodiscard]] Device build();

      private:
         template <PhysicaDeviceFeatureStruct FeatureStruct>
         static void enable_features(FeatureStruct& target, FeatureStruct const& source)
         {
            std::size_t offset{};
            if (not std::is_same_v<FeatureStruct, vk::PhysicalDeviceFeatures>)
               offset = alignof(FeatureStruct) * 2;

            std::size_t const feature_count{ (sizeof(FeatureStruct::NativeType) - offset) / sizeof(VkBool32) };
            auto const source_features{
               reinterpret_cast<vk::Bool32 const*>(reinterpret_cast<std::byte const*>(&source) + offset)
            };
            auto const target_features{
               reinterpret_cast<vk::Bool32* const>(reinterpret_cast<std::byte* const>(&target) + offset)
            };

            for (std::size_t index{}; index < feature_count; ++index)
               if (not target_features[index] and source_features[index])
                  target_features[index] = true;
         }

         template <PhysicaDeviceFeatureStruct FeatureStruct>
         [[nodiscard]] static bool any_requested_feature_missing(FeatureStruct const& requested, FeatureStruct const& available)
         {
            std::size_t offset{};
            if (not std::is_same_v<FeatureStruct, vk::PhysicalDeviceFeatures>)
               offset = alignof(FeatureStruct) * 2;

            std::size_t const feature_count{ (sizeof(FeatureStruct::NativeType) - offset) / sizeof(VkBool32) };
            auto const requested_features{
               reinterpret_cast<vk::Bool32 const*>(reinterpret_cast<std::byte const*>(&requested) + offset)
            };
            auto const available_features{
               reinterpret_cast<vk::Bool32 const*>(reinterpret_cast<std::byte const*>(&available) + offset)
            };

            for (std::size_t index{}; index < feature_count; ++index)
               if (requested_features[index] and not available_features[index])
                  return true;

            return false;
         }

         [[nodiscard]] static UniquePointer<VmaAllocator_T> create_allocator(vk::raii::PhysicalDevice const& physical_device,
            vk::raii::Device const& device);
         [[nodiscard]] vk::raii::PhysicalDevice pick_physical_device();
         [[nodiscard]] vk::raii::Device create_device(vk::raii::PhysicalDevice const& physical_device);
         [[nodiscard]] std::vector<DeviceQueue> retrieve_queues(vk::raii::Device const& device);
         [[nodiscard]] std::unordered_map<std::uint32_t, vk::raii::CommandPool> create_command_pools(
            vk::raii::Device const& device);

         std::set<std::string> extension_names_{};
         vk::PhysicalDeviceVulkan13Features features13_{};
         vk::PhysicalDeviceVulkan12Features features12_{ .pNext{ &features13_ } };
         vk::PhysicalDeviceVulkan11Features features11_{ .pNext{ &features12_ } };
         vk::PhysicalDeviceFeatures features10_{};
         vk::PhysicalDeviceFeatures2 features_{ .pNext{ &features11_ } };
         std::vector<RequiredQueueInfo> required_queues_{};

         QueueInfo queue_info_{};
   };
}

#endif