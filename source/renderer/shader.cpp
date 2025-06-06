#include "shader.hpp"
#include "utility/exception.hpp"

namespace eru
{
   shaderc::Compiler const Shader::COMPILER{};

   Shader::Shader(std::filesystem::path const& path, Device const& device)
      : module_{
         [&path, &device]
         {
            std::string const file_name{ path.filename().string() };
            if (not std::filesystem::exists(path))
               exception(std::format("{} does not exist!", file_name));

            if (std::ifstream const file{ path, std::ifstream::in }; file.is_open())
            {
               std::ostringstream code{};
               code << file.rdbuf();

               shaderc_shader_kind shader_type;

               if (std::string const file_type{ path.extension().string() }; file_type == ".glsl")
                  shader_type = shaderc_glsl_infer_from_source;
               else if (file_type == ".comp")
                  shader_type = shaderc_compute_shader;
               else if (file_type == ".frag")
                  shader_type = shaderc_fragment_shader;
               else if (file_type == ".geom")
                  shader_type = shaderc_geometry_shader;
               else if (file_type == ".mesh")
                  shader_type = shaderc_mesh_shader;
               else if (file_type == ".rahit")
                  shader_type = shaderc_anyhit_shader;
               else if (file_type == ".rcall")
                  shader_type = shaderc_callable_shader;
               else if (file_type == ".rchit")
                  shader_type = shaderc_closesthit_shader;
               else if (file_type == ".rgen")
                  shader_type = shaderc_raygen_shader;
               else if (file_type == ".rint")
                  shader_type = shaderc_intersection_shader;
               else if (file_type == ".rmiss")
                  shader_type = shaderc_miss_shader;
               else if (file_type == ".task")
                  shader_type = shaderc_task_shader;
               else if (file_type == ".tesc")
                  shader_type = shaderc_tess_control_shader;
               else if (file_type == ".tese")
                  shader_type = shaderc_tess_evaluation_shader;
               else if (file_type == ".vert")
                  shader_type = shaderc_vertex_shader;
               else
                  exception(std::format("\"{}\" is not a known shader file extension!", file_type));

               shaderc::CompilationResult const result{
                  COMPILER.CompileGlslToSpv(code.str(), shader_type, file_name.c_str())
               };

               if (result.GetCompilationStatus() not_eq shaderc_compilation_status_success)
               {
                  if (result.GetCompilationStatus() == shaderc_compilation_status_invalid_stage and
                     shader_type == shaderc_glsl_infer_from_source)
                     exception("cannot detect the shader type; specify it in the source code!");

                  exception(
                     std::format(
                        "failed to compile \"{}\"!\ncompilation status: {}\nerror: {}",
                        file_name,
                        static_cast<int>(result.GetCompilationStatus()),
                        result.GetErrorMessage()));
               }

               std::vector<std::uint32_t> byte_code{ result.begin(), result.end() };

               return device.device().createShaderModule({
                  .codeSize{ sizeof(std::uint32_t) * byte_code.size() },
                  .pCode{ byte_code.data() }
               });
            }
            exception(std::format("failed to open \"{}\"!", path.string()));
         }()
      }
   {
   }

   vk::raii::ShaderModule const& Shader::module() const
   {
      return module_;
   }
}