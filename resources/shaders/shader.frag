#version 450

layout(set = 1, binding = 0) uniform sampler2D texture_sampler[24];
layout(push_constant) uniform PushConstants {
   uint index;
} push;

layout(location = 0) in vec3 fragment_color;
layout(location = 1) in vec2 fragment_uv;

layout(location = 0) out vec4 color;

void main()
{
   color = vec4(fragment_color * texture(texture_sampler[push.index], fragment_uv).rgb, 1.0);
}