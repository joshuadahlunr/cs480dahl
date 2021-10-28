#version 330
// Position of the vertex
layout (location = 0) in vec3 pos;

// View and projection matricies (model not needed since our skybox cube should always surround the camera)
uniform mat4 viewMat;
uniform mat4 projectionMat;

// Output texture coordinates
out vec3 textureCoords;

void main() {
  // Cube map texture coordinates are just the position of the cubed
  textureCoords = pos;
  // Apply the VP matricies to get final position
  gl_Position = projectionMat * viewMat * vec4(pos, 1.0);
}
