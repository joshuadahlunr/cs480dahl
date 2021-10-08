#version 330

// Load the position, color, and texture coordinates from the CPU
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_uv;
// Load the MVP matricies from the CPU
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// We output color and textures coordinates
smooth out vec3 color;
smooth out vec2 uv;

void main(void) {
  // Make the position we loaded homogenious and apply MVP
  vec4 v = vec4(v_position, 1.0);
  gl_Position = (projectionMatrix * viewMatrix * modelMatrix) * v;
  // Output the color and texture coordinates
  color = v_color;
  uv = v_uv;
}
