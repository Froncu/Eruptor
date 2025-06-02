#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
   float z = gl_FragCoord.z;
   z = pow(z, 20000.0);
   outColor = vec4(vec3(z), 1.0);
}