#version 450

layout(set = 0, binding = 0) uniform Camera {
   mat4 view;
   mat4 projection;
} camera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragment_color;
layout(location = 1) out vec2 fragment_uv;

void main()
{
   gl_Position =
      camera.projection *
      camera.view *
      vec4(position, 1.0);

   fragment_color = color;
   fragment_uv = uv;
}