#ifndef LUMINANCE_PASS_HPP
#define LUMINANCE_PASS_HPP

#include "renderer/buffer.hpp"
#include "renderer/descriptor_sets.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/shader.hpp"

namespace eru
{
   class LuminancePass final
   {
      public:
         LuminancePass(Device const& device, DescriptorSets const& descciptor_sets, std::uint32_t frames_in_flight);

      private:
         DescriptorSets const& descriptor_sets_;

         Shader histogram_shader_;
         Pipeline histogram_pipeline_;
         Shader avarage_luminance_shader_;
         Pipeline avarage_luminance_pipeline_;

         Buffer histogram_buffer_;
         Buffer avarage_luminance_buffer_;
   };
}

#endif