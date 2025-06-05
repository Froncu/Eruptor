#version 450

layout(set = 0, binding = 0) uniform Camera {
   mat4 view;
   mat4 projection;
} camera;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;

void main()
{
   const vec4 world_position = vec4(in_position, 1.0);
   gl_Position = camera.projection * camera.view * world_position;

   out_uv = in_uv;
}
