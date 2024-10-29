#ifndef SHADER_COMPILER_HPP
#define SHADER_COMPILER_HPP

namespace eru
{
   [[nodiscard]] std::vector<std::uint32_t> compile_shader(std::filesystem::path const& path);
}

#endif