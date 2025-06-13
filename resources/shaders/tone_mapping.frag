#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_samplerless_texture_functions : require

layout(push_constant) uniform PushConstants {
   uint current_frame;
} push_constants;

layout(set = 0, binding = 0) uniform sampler texture_sampler;
layout(set = 0, binding = 1) uniform texture2D hdr_textures[];
layout(set = 1, binding = 0) readonly buffer Luminance{
   float value;
} luminance;

layout(location = 0) out vec4 out_color;

float ev100_from_luminance(float luminance)
{
   const float k = 12.5;
   return log2((luminance * 100) / k);
}

float ev100_to_exposure(float ev100)
{
   const float max_luminance = 1.2 * pow(2.0, ev100);
   return 1.0 / max(max_luminance, 0.0001);
}

// Uncharted 2 tone map curve
vec3 tone_map_curve(in vec3 color)
{
   const float a = 0.15;
   const float b = 0.50;
   const float c = 0.10;
   const float d = 0.20;
   const float e = 0.02;
   const float f = 0.30;
   return ((color * (a * color + c * b) + d * e)
   / (color * (a * color + b) + d * f))
   - e / f;
}

// Uncharted 2 tone map
vec3 tone_map(in vec3 color)
{
   const float white_point = 11.2;
   const vec3 curved_color = tone_map_curve(color);
   float white_scale = 1.0 / tone_map_curve(vec3(white_point)).r;
   return clamp(curved_color * white_scale, 0.0, 1.0);
}

void main()
{
   const uint current_frame = push_constants.current_frame;
   const ivec2 uv = ivec2(gl_FragCoord.xy);
   
   const vec3 hdr_color = texelFetch(sampler2D(hdr_textures[current_frame], texture_sampler), uv, 0).rgb;
   const float exposure = ev100_to_exposure(ev100_from_luminance(luminance.value));
   out_color = vec4(tone_map(hdr_color * exposure), 1.0);
}