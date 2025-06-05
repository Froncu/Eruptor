#version 450

layout(set = 0, binding = 0) uniform Camera {
   mat4 view;
   mat4 projection;
} camera;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_normal;
layout(location = 3) out vec3 out_tangent;
layout(location = 4) out vec3 out_bitangent;

void main()
{
   gl_Position = camera.projection * camera.view * vec4(in_position, 1.0);
   out_position = in_position;
   out_uv = in_uv;
   out_normal = in_normal;
   out_tangent = in_tangent;
   out_bitangent = in_bitangent;
}