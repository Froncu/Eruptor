#version 450

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) in vec3 fragment_color;
layout(location = 1) in vec2 fragment_uv;

layout(location = 0) out vec4 color;

void main()
{
   color = vec4(fragment_color * texture(texture_sampler, fragment_uv).rgb, 1.0);
}