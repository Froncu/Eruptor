#version 450

layout(binding = 0) uniform Camera {
   mat4 view;
   mat4 projection;
} camera;

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragColor;

void main() {
   gl_Position =
      camera.projection *
      camera.view *
      vec4(position, 1.0);
   
   fragColor = vec3(0.5, 0.5, 0.5);
}