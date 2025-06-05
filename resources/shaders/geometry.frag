#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct Material {
   int base_color_index;
   int normal_index;
   int metalness_index;
};

layout(push_constant) uniform PushConstants {
   uint material_index;
} push_constants;

layout(set = 1, binding = 1) uniform sampler texture_sampler;
layout(set = 1, binding = 2) uniform texture2D base_color_textures[];
layout(set = 1, binding = 3) uniform texture2D normal_textures[];
layout(set = 1, binding = 4) uniform texture2D metalness_textures[];
layout(set = 1, binding = 0) readonly buffer MaterialsBuffer{
   Material materials[];
} materials_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec4 out_color;
layout(location = 2) out vec4 out_normal;
layout(location = 3) out vec4 out_metalness;

void main()
{
   out_position = vec4(in_position, 1.0);
   
   const Material material = materials_buffer.materials[push_constants.material_index];

   const int base_color_index = material.base_color_index;
   const int normal_index = material.normal_index;
   const int metalness_index = material.metalness_index;

   // Base color
   if (base_color_index >= 0)
      out_color = texture(sampler2D(base_color_textures[base_color_index], texture_sampler), in_uv);
   else
      out_color = vec4(1.0, 1.0, 1.0, 1.0); // TODO: use the interpolated vertex color

   // Normal
   if (normal_index >= 0)
   {
      const mat3 tbn = mat3(normalize(in_tangent), normalize(in_bitangent), normalize(in_normal));
      vec3 normal_map = texture(sampler2D(normal_textures[normal_index], texture_sampler), in_uv).rgb;
      normal_map = normal_map * 2.0 - 1;
      out_normal = vec4(tbn * normal_map, 1.0);
   } 
   else
      out_normal = vec4(in_normal, 1.0);

   // Metalness
   if (metalness_index >= 0)
      out_metalness = texture(sampler2D(metalness_textures[metalness_index], texture_sampler), in_uv);
   else
      out_metalness = vec4(0.0, 0.0, 0.0, 1.0); // TODO: use the interpolated metalness value
}