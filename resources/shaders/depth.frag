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
layout(set = 1, binding = 0) readonly buffer MaterialsBuffer{
   Material materials[];
} materials_buffer;

layout(location = 0) in vec2 in_uv;

void main()
{
   const Material material = materials_buffer.materials[push_constants.material_index];

   const int base_color_index = material.base_color_index;
   
   const float alpha = texture(sampler2D(base_color_textures[base_color_index], texture_sampler), in_uv).a;
   if (alpha < 0.5) discard;
}
