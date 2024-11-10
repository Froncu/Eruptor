#include "erupch.hpp"

#include "shader_compiler.hpp"

namespace eru
{
   std::vector<std::uint32_t> compile_shader(std::filesystem::path const& path)
   {
      static shaderc::Compiler const compiler{};

      std::string const file_name{ path.filename().string() };
      if (not std::filesystem::exists(path))
         throw std::runtime_error(std::format("{} does not exist!", file_name));

      if (std::ifstream const file{ path, std::ifstream::in }; file.is_open())
      {
         std::ostringstream code{};
         code << file.rdbuf();

         shaderc_shader_kind shader_type;
         std::string file_type{ path.extension().string() };

         if (file_type == ".glsl")
            shader_type = shaderc_shader_kind::shaderc_glsl_infer_from_source;
         else if (file_type == ".comp")
            shader_type = shaderc_shader_kind::shaderc_compute_shader;
         else if (file_type == ".frag")
            shader_type = shaderc_shader_kind::shaderc_fragment_shader;
         else if (file_type == ".geom")
            shader_type = shaderc_shader_kind::shaderc_geometry_shader;
         else if (file_type == ".mesh")
            shader_type = shaderc_shader_kind::shaderc_mesh_shader;
         else if (file_type == ".rahit")
            shader_type = shaderc_shader_kind::shaderc_anyhit_shader;
         else if (file_type == ".rcall")
            shader_type = shaderc_shader_kind::shaderc_callable_shader;
         else if (file_type == ".rchit")
            shader_type = shaderc_shader_kind::shaderc_closesthit_shader;
         else if (file_type == ".rgen")
            shader_type = shaderc_shader_kind::shaderc_raygen_shader;
         else if (file_type == ".rint")
            shader_type = shaderc_shader_kind::shaderc_intersection_shader;
         else if (file_type == ".rmiss")
            shader_type = shaderc_shader_kind::shaderc_miss_shader;
         else if (file_type == ".task")
            shader_type = shaderc_shader_kind::shaderc_task_shader;
         else if (file_type == ".tesc")
            shader_type = shaderc_shader_kind::shaderc_tess_control_shader;
         else if (file_type == ".tese")
            shader_type = shaderc_shader_kind::shaderc_tess_evaluation_shader;
         else if (file_type == ".vert")
            shader_type = shaderc_shader_kind::shaderc_vertex_shader;
         else
            throw std::runtime_error(std::format("\"{}\" is not a known shader file extension!", file_type));

         shaderc::CompilationResult const result{
            compiler.CompileGlslToSpv(code.str(), shader_type, file_name.c_str())
         };

         if (result.GetCompilationStatus() not_eq shaderc_compilation_status::shaderc_compilation_status_success)
         {
            if (result.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_invalid_stage and
               shader_type == shaderc_shader_kind::shaderc_glsl_infer_from_source)
               throw std::runtime_error("cannot detect the shader type; specify it in the source code!");

            throw std::runtime_error(
               std::format(
                  "failed to compile \"{}\"!\ncompilation status: {}\nerror: {}",
                  file_name,
                  static_cast<int>(result.GetCompilationStatus()),
                  result.GetErrorMessage()));
         }

         return { result.begin(), result.end() };
      }
      throw std::runtime_error(std::format("failed to open \"{}\"!", path.string()));
   }
}