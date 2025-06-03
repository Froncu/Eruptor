#version 450

layout(push_constant) uniform PushConstants {
   uint index;
} push;

layout(set = 1, binding = 0) uniform sampler texture_sampler;
layout(set = 1, binding = 1) uniform texture2D textures[25];

layout(location = 0) in vec3 fragment_color;
layout(location = 1) in vec2 fragment_uv;

layout(location = 0) out vec4 color;

void main()
{
   color = vec4(fragment_color *
      texture(sampler2D(textures[push.index], texture_sampler), fragment_uv).rgb, 1.0);
}