#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct Material {
   int diffuse_index;
};

layout(push_constant) uniform PushConstants {
   int material_index;
} push_constants;

layout(set = 1, binding = 1) uniform sampler texture_sampler;
layout(set = 1, binding = 2) uniform texture2D textures[];
layout(set = 1, binding = 0) readonly buffer MaterialsBuffer{
   Material materials[];
} materials_buffer;

layout(location = 0) in vec2 fragment_uv;

layout(location = 0) out vec4 color;

void main()
{
   const Material material = materials_buffer.materials[push_constants.material_index];

   const int diffuse_index = material.diffuse_index;
   
   color.rgb = texture(sampler2D(textures[diffuse_index], texture_sampler), fragment_uv).rgb;
   color.a = 1.0;
}