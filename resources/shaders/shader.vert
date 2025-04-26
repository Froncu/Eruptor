#version 450

layout(binding = 0) uniform UniformBufferObject {
   mat4 model;
   mat4 view;
   mat4 projection;
} uniform_buffer_object;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragment_color;

void main()
{
   gl_Position =
      uniform_buffer_object.projection *
      uniform_buffer_object.view *
      uniform_buffer_object.model *
      vec4(position, 1.0);

   fragment_color = color;
}