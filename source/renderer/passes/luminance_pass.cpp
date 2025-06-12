#include "luminance_pass.hpp"
#include "builders/buffer_builder.hpp"
#include "builders/compute_pipeline_builder.hpp"

namespace eru
{
   LuminancePass::LuminancePass(Device const& device, DescriptorSets const& descciptor_sets, std::uint32_t frames_in_flight)
      : descriptor_sets_{ descciptor_sets }
      , histogram_shader_{ "", device }
      , histogram_pipeline_{
         ComputePipelineBuilder{}
         .change_shader_stage({
            .stage{ vk::ShaderStageFlagBits::eCompute },
            .module{ *histogram_shader_.module() },
            .pName{ "main" }
         })
         .assign_descriptor_set_layout("histogram", 0)
         .build(device, descriptor_sets_)
      }
      , avarage_luminance_shader_{ "", device }
      , avarage_luminance_pipeline_{
         ComputePipelineBuilder{}
         .change_shader_stage({
            .stage{ vk::ShaderStageFlagBits::eCompute },
            .module{ *avarage_luminance_shader_.module() },
            .pName{ "main" }
         })
         .assign_descriptor_set_layout("histogram", 1)
         .assign_descriptor_set_layout("avarage_luminance", 1)
         .build(device, descriptor_sets_)
      }
      , histogram_buffer_{
         BufferBuilder{}
         .change_size(256 * sizeof(std::uint32_t) * frames_in_flight)
         .change_usage(vk::BufferUsageFlagBits::eStorageBuffer)
         .change_sharing_mode(vk::SharingMode::eExclusive)
         .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
         .build(device)
      }
      , avarage_luminance_buffer_{
         BufferBuilder{}
         .change_size(sizeof(float) * frames_in_flight)
         .change_usage(vk::BufferUsageFlagBits::eStorageBuffer)
         .change_sharing_mode(vk::SharingMode::eExclusive)
         .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
         .build(device)
      }
   {
   }
}