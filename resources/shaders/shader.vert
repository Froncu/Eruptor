#version 450

layout(set = 0, binding = 0) uniform Camera {
   mat4 view;
   mat4 projection;
} camera;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_tangent;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out mat3 out_tbn;

void main()
{
   gl_Position = camera.projection * camera.view * vec4(in_position, 1.0);
   
   out_uv = in_uv;

   const vec3 tangent = normalize(in_tangent);
   const vec3 normal = normalize(in_normal);
   const vec3 bitangent = normalize(cross(normal, tangent));
   out_tbn = mat3(tangent, bitangent, normal);
}