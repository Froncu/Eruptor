#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "device.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include "swap_chain.hpp"
#include "window/window.hpp"

namespace eru
{
   class Renderer final
   {
      public:
         explicit Renderer(Window const& window);
         Renderer(Renderer const&) = delete;
         Renderer(Renderer&&) = delete;

         ~Renderer() = default;

         Renderer& operator=(Renderer const&) = delete;
         Renderer& operator=(Renderer&&) = delete;

         void render();

      private:
         Device device_;
         SwapChain swap_chain_;
         Shader vertex_shader_{ "resources/shaders/shader.vert", device_ };
         Shader fragment_shader_{ "resources/shaders/shader.frag", device_ };
         Pipeline pipeline_;
   };
}

#endif