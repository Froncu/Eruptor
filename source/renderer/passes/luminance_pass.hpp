#ifndef LUMINANCE_PASS_HPP
#define LUMINANCE_PASS_HPP

#include "builders/buffer_builder.hpp"
#include "renderer/buffer.hpp"
#include "renderer/descriptor_sets.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/shader.hpp"

namespace eru
{
   class LuminancePass final
   {
      struct HistogramPushConstants final
      {
         std::uint32_t current_frame;
         float minimal_log_luminance;
         float inverse_log_luminance_range;
      };

      struct AvarageLuminancePushConstants final
      {
         std::uint32_t current_frame;
         float minimal_log_luminance;
         float log_luminance_range;
         float time_coefficient;
         float delta_seconds;
      };

      public:
         struct Settings final
         {
            float minimal_log_luminance{ -4.0f };
            float maximal_log_luminance{ 16.0f };
            float time_coefficient{ 0.01f };
         };

         LuminancePass(Device const& device, DescriptorSets const& descciptor_sets, std::uint32_t frames_in_flight);
         LuminancePass(LuminancePass const&) = delete;
         LuminancePass(LuminancePass&&) = default;

         ~LuminancePass() = default;

         LuminancePass& operator=(LuminancePass const&) = delete;
         LuminancePass& operator=(LuminancePass&&) = delete;

         void change_minimal_log_luminance(float minimal_log_luminance);
         void change_maximal_log_luminance(float maximal_log_luminance);
         void change_time_coefficient(float time_coefficient);

         void compute(vk::raii::CommandBuffer const& command_buffer, std::uint32_t current_frame, float delta_seconds) const;

      private:
         static std::uint32_t constexpr HISTOGRAM_BIN_COUNT{ 256 };

         DescriptorSets const& descriptor_sets_;
         std::uint32_t const frames_in_flight_;

         Shader histogram_shader_;
         Pipeline histogram_pipeline_;
         Shader avarage_luminance_shader_;
         Pipeline avarage_luminance_pipeline_;

         std::vector<Buffer> histogram_buffers_;
         std::vector<Buffer> avarage_luminance_buffers_;

         Settings settings_{};
   };
}

#endif