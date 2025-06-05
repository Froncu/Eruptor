#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_samplerless_texture_functions : require

layout(push_constant) uniform PushConstants {
   uint current_frame;
} push_constants;

layout(set = 0, binding = 0) uniform sampler texture_sampler;
layout(set = 0, binding = 1) uniform texture2D position_textures[];
layout(set = 0, binding = 2) uniform texture2D base_color_textures[];
layout(set = 0, binding = 3) uniform texture2D normal_textures[];
layout(set = 0, binding = 4) uniform texture2D metalness_textures[];

layout(location = 0) out vec4 out_color;

void main()
{
   const uint current_frame = push_constants.current_frame;
   const vec2 uv = gl_FragCoord.xy / vec2(textureSize(base_color_textures[current_frame], 0));
   out_color = vec4(texture(sampler2D(base_color_textures[current_frame], texture_sampler), uv).rgb, 1.0);
}