#include "luminance_pass.hpp"
#include "luminance_pass.hpp"
#include "builders/compute_pipeline_builder.hpp"

namespace eru
{
   LuminancePass::LuminancePass(Device const& device, DescriptorSets const& descciptor_sets,
      std::uint32_t const frames_in_flight)
      : descriptor_sets_{ descciptor_sets }
      , frames_in_flight_{ frames_in_flight }
      , histogram_shader_{ "resources/shaders/histogram.comp", device }
      , histogram_pipeline_{
         ComputePipelineBuilder{}
         .change_shader_stage({
            .stage{ vk::ShaderStageFlagBits::eCompute },
            .module{ *histogram_shader_.module() },
            .pName{ "main" }
         })
         .assign_descriptor_set_layout("hdr", 0)
         .assign_descriptor_set_layout("histogram", 1)
         .add_push_constant_range({
            .stageFlags{ vk::ShaderStageFlagBits::eCompute },
            .size{ sizeof(HistogramPushConstants) }
         })
         .build(device, descriptor_sets_)
      }
      , avarage_luminance_shader_{ "resources/shaders/avarage_luminance.comp", device }
      , avarage_luminance_pipeline_{
         ComputePipelineBuilder{}
         .change_shader_stage({
            .stage{ vk::ShaderStageFlagBits::eCompute },
            .module{ *avarage_luminance_shader_.module() },
            .pName{ "main" }
         })
         .assign_descriptor_set_layout("hdr", 0)
         .assign_descriptor_set_layout("histogram", 1)
         .assign_descriptor_set_layout("avarage_luminance", 2)
         .assign_descriptor_set_layout("avarage_luminance", 3)
         .add_push_constant_range({
            .stageFlags{ vk::ShaderStageFlagBits::eCompute },
            .size{ sizeof(AvarageLuminancePushConstants) }
         })
         .build(device, descriptor_sets_)
      }
      , histogram_buffers_{
         [this, &device]
         {
            auto const buffer_builder{
               BufferBuilder{}
               .change_size(sizeof(std::uint32_t) * HISTOGRAM_BIN_COUNT)
               .change_usage(vk::BufferUsageFlagBits::eStorageBuffer)
               .change_sharing_mode(vk::SharingMode::eExclusive)
               .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
            };

            std::vector<Buffer> buffers{};
            for (std::uint32_t index{}; index < frames_in_flight_; ++index)
               buffers.emplace_back(buffer_builder.build(device));

            return buffers;
         }()
      }
      , avarage_luminance_buffers_{
         [this, &device]
         {
            auto const buffer_builder{
               BufferBuilder{}
               .change_size(sizeof(float))
               .change_usage(vk::BufferUsageFlagBits::eStorageBuffer)
               .change_sharing_mode(vk::SharingMode::eExclusive)
               .change_allocation_required_flags(vk::MemoryPropertyFlagBits::eDeviceLocal)
            };

            std::vector<Buffer> buffers{};
            for (std::uint32_t index{}; index < frames_in_flight_; ++index)
               buffers.emplace_back(buffer_builder.build(device));

            return buffers;
         }()
      }
   {
      std::vector<vk::DescriptorBufferInfo> infos{};
      infos.reserve(frames_in_flight_ * 2);
      std::vector<vk::WriteDescriptorSet> writes{};
      writes.reserve(frames_in_flight_ * 2);

      for (std::uint32_t index{}; index < frames_in_flight_; ++index)
      {
         infos.push_back({
            .buffer{ histogram_buffers_[index].buffer() },
            .range{ sizeof(std::uint32_t) * HISTOGRAM_BIN_COUNT }
         });

         writes.push_back({
            .dstSet{ *descriptor_sets_.sets("histogram")[index] },
            .dstBinding{ descriptor_sets_.binding("histogram", "bins") },
            .descriptorCount{ 1 },
            .descriptorType{ vk::DescriptorType::eStorageBuffer },
            .pBufferInfo{ &infos.back() }
         });

         infos.push_back({
            .buffer{ avarage_luminance_buffers_[index].buffer() },
            .range{ sizeof(float) }
         });

         writes.push_back({
            .dstSet{ *descriptor_sets_.sets("avarage_luminance")[index] },
            .dstBinding{ descriptor_sets_.binding("avarage_luminance", "value") },
            .descriptorCount{ 1 },
            .descriptorType{ vk::DescriptorType::eStorageBuffer },
            .pBufferInfo{ &infos.back() }
         });
      }

      device.device().updateDescriptorSets(writes, {});
   }

   void LuminancePass::change_minimal_log_luminance(float const minimal_log_luminance)
   {
      settings_.minimal_log_luminance = minimal_log_luminance;
   }

   void LuminancePass::change_maximal_log_luminance(float const maximal_log_luminance)
   {
      settings_.maximal_log_luminance = maximal_log_luminance;
   }

   void LuminancePass::change_time_coefficient(float const time_coefficient)
   {
      settings_.time_coefficient = time_coefficient;
   }

   void LuminancePass::compute(vk::raii::CommandBuffer const& command_buffer, std::uint32_t const current_frame,
      float const delta_seconds) const
   {
      // TODO: is this needed?
      vk::BufferMemoryBarrier2 const histogram_begin_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eComputeShader },
         .dstAccessMask{ vk::AccessFlagBits2::eShaderStorageWrite },
         .buffer{ histogram_buffers_[current_frame].buffer() },
         .size{ sizeof(std::uint32_t) * HISTOGRAM_BIN_COUNT }
      };

      command_buffer.pipelineBarrier2({
         .bufferMemoryBarrierCount{ 1 },
         .pBufferMemoryBarriers{ &histogram_begin_barrier }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, histogram_pipeline_.pipeline());

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, histogram_pipeline_.layout(), 0,
         {
            descriptor_sets_.sets("hdr").front(),
            descriptor_sets_.sets("histogram")[current_frame]
         }, {});

      float const log_luminance_range{ settings_.maximal_log_luminance - settings_.minimal_log_luminance };

      command_buffer.pushConstants<HistogramPushConstants>(
         histogram_pipeline_.layout(),
         vk::ShaderStageFlagBits::eCompute,
         0,
         HistogramPushConstants{
            .current_frame{ current_frame },
            .minimal_log_luminance{ settings_.minimal_log_luminance },
            .inverse_log_luminance_range{ 1.0f / log_luminance_range }
         });

      // TODO: this should be based on the size of the input image.
      command_buffer.dispatch(128, 128, 1);

      vk::BufferMemoryBarrier2 const histogram_end_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eComputeShader },
         .srcAccessMask{ vk::AccessFlagBits2::eShaderStorageWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eComputeShader },
         .dstAccessMask{ vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite },
         .buffer{ histogram_buffers_[current_frame].buffer() },
         .size{ sizeof(std::uint32_t) * HISTOGRAM_BIN_COUNT }
      };

      // TODO: is this needed?
      vk::BufferMemoryBarrier2 const avarage_luminance_begin_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eNone },
         .srcAccessMask{ vk::AccessFlagBits2::eNone },
         .dstStageMask{ vk::PipelineStageFlagBits2::eComputeShader },
         .dstAccessMask{ vk::AccessFlagBits2::eShaderStorageWrite },
         .buffer{ avarage_luminance_buffers_[current_frame].buffer() },
         .size{ sizeof(float) }
      };

      std::array const barriers{
         histogram_end_barrier,
         avarage_luminance_begin_barrier
      };

      command_buffer.pipelineBarrier2({
         .bufferMemoryBarrierCount{ static_cast<std::uint32_t>(barriers.size()) },
         .pBufferMemoryBarriers{ barriers.data() }
      });

      command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, avarage_luminance_pipeline_.pipeline());

      command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, avarage_luminance_pipeline_.layout(), 0,
         {
            descriptor_sets_.sets("hdr").front(),
            descriptor_sets_.sets("histogram")[current_frame],
            descriptor_sets_.sets("avarage_luminance")[(current_frame + frames_in_flight_ - 1) % frames_in_flight_],
            descriptor_sets_.sets("avarage_luminance")[current_frame]
         }, {});

      command_buffer.pushConstants<AvarageLuminancePushConstants>(
         avarage_luminance_pipeline_.layout(),
         vk::ShaderStageFlagBits::eCompute,
         0,
         AvarageLuminancePushConstants{
            .current_frame{ current_frame },
            .minimal_log_luminance{ settings_.minimal_log_luminance },
            .log_luminance_range{ log_luminance_range },
            .time_coefficient{ settings_.time_coefficient },
            .delta_seconds{ delta_seconds }
         });

      command_buffer.dispatch(1, 1, 1);

      vk::BufferMemoryBarrier2 const avarage_luminance_end_barrier{
         .srcStageMask{ vk::PipelineStageFlagBits2::eComputeShader },
         .srcAccessMask{ vk::AccessFlagBits2::eShaderStorageWrite },
         .dstStageMask{ vk::PipelineStageFlagBits2::eFragmentShader },
         .dstAccessMask{ vk::AccessFlagBits2::eShaderStorageRead },
         .buffer{ avarage_luminance_buffers_[current_frame].buffer() },
         .size{ sizeof(float) }
      };

      command_buffer.pipelineBarrier2({
         .bufferMemoryBarrierCount{ 1 },
         .pBufferMemoryBarriers{ &avarage_luminance_end_barrier }
      });
   }
}