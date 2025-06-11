#ifndef TONE_MAP_PASS_HPP
#define TONE_MAP_PASS_HPP

#include "renderer/descriptor_sets.hpp"
#include "renderer/image.hpp"
#include "renderer/image_view.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/shader.hpp"

namespace eru
{
   class ToneMapPass final
   {
      public:
         ToneMapPass(Device const& device, vk::Format swap_chain_format, DescriptorSets const& descriptor_sets);
         ToneMapPass(ToneMapPass const&) = delete;
         ToneMapPass(ToneMapPass&&) = default;

         ~ToneMapPass() = default;

         ToneMapPass& operator=(ToneMapPass const&) = delete;
         ToneMapPass& operator=(ToneMapPass&&) = delete;

         void render(vk::raii::CommandBuffer const& command_buffer, Image const& swap_chain_image,
            ImageView const& swap_chain_image_view, vk::Extent2D swap_chain_extent, std::uint32_t current_frame) const;

      private:
         DescriptorSets const& descriptor_sets_;

         Shader vertex_shader_;
         Shader fragment_shader_;
         Pipeline pipeline_;
   };
}

#endif