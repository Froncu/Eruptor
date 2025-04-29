#ifndef RENDER_PASS_HPP
#define RENDER_PASS_HPP

#include "erupch/erupch.hpp"

namespace eru
{
   class RenderPass final
   {
      public:
         RenderPass(RenderPass const&) = delete;
         RenderPass(RenderPass&&) = delete;

         ~RenderPass() = default;

         RenderPass& operator=(RenderPass const&) = delete;
         RenderPass& operator=(RenderPass&&) = delete;

      private:
         vk::raii::RenderPass render_pass_;
   };
}

#endif