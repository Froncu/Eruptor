#ifndef SHADER_HPP
#define SHADER_HPP

#include "erupch/erupch.hpp"
#include "renderer/device.hpp"

namespace eru
{
   class Shader final
   {
      public:
         Shader(std::filesystem::path const& path, Device const& device);
         Shader(Shader const&) = delete;
         Shader(Shader&&) = default;

         ~Shader() = default;

         Shader& operator=(Shader const&) = delete;
         Shader& operator=(Shader&&) = default;

         vk::raii::ShaderModule const& module() const;

      private:
         static shaderc::Compiler const COMPILER;

         vk::raii::ShaderModule module_;
   };
}

#endif