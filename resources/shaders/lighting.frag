#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_samplerless_texture_functions : require

const float pi = 3.14159265359;

// TODO: upload this data from the application
const int light_count = 3;
const vec3 point_lights[light_count] = vec3[](
    vec3(-5.0, 1.0, 0.2),
    vec3(0.0, 1.0, 0.2),
    vec3(5.0, 1.0, 0.2)
);
const vec3 point_light_colors[light_count] = vec3[](
   vec3(10.0, 1.0, 1.0),
   vec3(1.0, 10.0, 1.0),
   vec3(1.0, 1.0, 10.0)
);
const vec3 directional_lights[light_count] = vec3[](
    normalize(vec3(-0.5, -1.0, -0.3)),
    normalize(vec3(0.0, -1.0, 0.0)),
    normalize(vec3(0.5, -1.0, 0.3))
);
const vec3 directional_light_colors[light_count] = vec3[](
    vec3(1.2, 1.1, 1.0),
    vec3(1.0, 1.6, 2.0),
    vec3(2.5, 2.4, 2.2)
);
//const vec3 directional_light_colors[light_count] = vec3[](
//    vec3(60.0, 56.0, 50.0),   // Warm white (incandescent)
//    vec3(50.0, 80.0, 100.0),  // Cool white (daylight)
//    vec3(255.0, 241.0, 224.0) // Sunlight (D65)
//);

layout(push_constant) uniform PushConstants {
   vec3 camera_position;
   uint current_frame;
} push_constants;

layout(set = 0, binding = 0) uniform sampler texture_sampler;
layout(set = 0, binding = 1) uniform texture2D position_textures[];
layout(set = 0, binding = 2) uniform texture2D base_color_textures[];
layout(set = 0, binding = 3) uniform texture2D normal_textures[];
layout(set = 0, binding = 4) uniform texture2D metalness_textures[];

layout(location = 0) out vec4 out_color;

vec3 fresnel_schlick(float cosine_theta, vec3 base_reflectability)
{
   return base_reflectability + (1.0 - base_reflectability) * pow(clamp(1.0 - cosine_theta, 0.0, 1.0), 5.0);
}

float distribution_ggx(vec3 n, vec3 h, float roughness)
{
   const float a = roughness * roughness;
   const float n_dot_h = max(dot(n, h), 0.0);
   const float n_dot_h_squared = n_dot_h * n_dot_h;
   
   float denominator = (n_dot_h_squared * (a - 1.0) + 1.0);
   denominator = pi * denominator * denominator;
   
   return a / denominator;
}

float geometry_schlick_ggx_direct(float n_dot_v, float roughness)
{
   const float r = roughness + 1.0;
   const float k = r * r / 8.0;
   
   return n_dot_v / (n_dot_v * (1.0 - k) + k);
}

float geometry_schlick_ggx_indirect(float n_dot_v, float roughness)
{
   const float k = roughness * roughness / 8.0;
   
   return n_dot_v / (n_dot_v * (1.0 - k) + k);
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness, bool direct)
{
   const float n_dot_l = max(dot(n, l), 0.0);
   const float n_dot_v = max(dot(n, v), 0.0);
   
   if (direct)
      return
         geometry_schlick_ggx_direct(n_dot_v, roughness) *
         geometry_schlick_ggx_direct(n_dot_l, roughness);
   else
      return
         geometry_schlick_ggx_indirect(n_dot_v, roughness) *
         geometry_schlick_ggx_indirect(n_dot_l, roughness);
}

void main()
{
   const uint current_frame = push_constants.current_frame;
   const ivec2 uv = ivec2(gl_FragCoord.xy);

   const vec3 position = texelFetch(sampler2D(position_textures[current_frame], texture_sampler), uv, 0).rgb;
   vec3 base_color = pow(texelFetch(sampler2D(base_color_textures[current_frame], texture_sampler), uv, 0).rgb, vec3(2.2));
   const vec3 n = texelFetch(sampler2D(normal_textures[current_frame], texture_sampler), uv, 0).rgb;
   const vec3 metalness = texelFetch(sampler2D(metalness_textures[current_frame], texture_sampler), uv, 0).rgb;
   const float metallic = metalness.b;
   const float roughness = metalness.g;
   
   const vec3 v = normalize(push_constants.camera_position - position);
   const vec3 base_reflectability = mix(vec3(0.04), base_color, metallic);

   vec3 lo = vec3(0.0);
   
   for (int index = 0; index < light_count; ++index)
   {
      const vec3 l = normalize(point_lights[index] - position);
      const vec3 h = normalize(v + l);

      const float distribution = distribution_ggx(n, h, roughness);
      const float geometry = geometry_smith(n, v, l, roughness, true);
      const vec3 fresnel = fresnel_schlick(max(dot(h, v), 0.0), base_reflectability);
      const vec3 numerator = distribution * geometry * fresnel;
      const float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001;
      const vec3 specular = numerator / denominator;

      const vec3 kd = (vec3(1.0) - fresnel) * (1.0 - metallic);

      const float distance = length(point_lights[index] - position);
      const float attenuation = 1.0 / (distance * distance);
      const vec3 radiance = point_light_colors[index] * attenuation;

      const float n_dot_l = max(dot(n, l), 0.0);

      lo += (specular + base_color * kd / pi) * radiance * n_dot_l;
   }

   for (int index = 0; index < light_count; ++index)
   {
      const vec3 l = normalize(point_lights[index]);
      const vec3 h = normalize(v + l);

      const float distribution = distribution_ggx(n, h, roughness);
      const float geometry = geometry_smith(n, v, l, roughness, true);
      const vec3 fresnel = fresnel_schlick(max(dot(h, v), 0.0), base_reflectability);
      const vec3 numerator = distribution * geometry * fresnel;
      const float denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001;
      const vec3 specular = numerator / denominator;

      const vec3 kd = (vec3(1.0) - fresnel) * (1.0 - metallic);

      const vec3 radiance = directional_light_colors[index];

      const float n_dot_l = max(dot(n, l), 0.0);

      lo += (specular + base_color * kd / pi) * radiance * n_dot_l;
   }

   const vec3 ambient = vec3(0.03) * base_color * 1.0;
   base_color = ambient + lo;

   base_color = base_color / (base_color + vec3(1.0));

   out_color = vec4(base_color, 1.0);
}