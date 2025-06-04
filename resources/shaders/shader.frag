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

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_position;
layout(location = 2) in mat3 in_tbn;

layout(location = 0) out vec4 out_color;

void main()
{
   const Material material = materials_buffer.materials[push_constants.material_index];

   const int base_color_index = material.base_color_index;
   const int normal_index = material.normal_index;
   const int metalness_index = material.metalness_index;

   if (base_color_index >= 0)
      out_color.rgb = texture(sampler2D(base_color_textures[base_color_index], texture_sampler), in_uv).rgb;
   else
      out_color.rgb = vec3(1.0, 0.0, 0.0);

   out_color.a = 1.0;
}