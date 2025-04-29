#include "render_pass.hpp"

namespace eru
{
   RenderPass::RenderPass(vk::raii::RenderPass render_pass)
      : render_pass_{ std::move(render_pass) }
   {
   }
}