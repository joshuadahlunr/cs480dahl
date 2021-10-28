#version 330

// Load the MVP matricies from the CPU
uniform vec4 ambientProduct, diffuseProduct, specularProduct;
uniform float shininess;
uniform vec4 lightPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// We input vertex color and textures coordinates
smooth in vec3 color;
smooth in vec2 uv;
// Input values that were interpolatated per-fragment
in vec3 fN;
in vec3 fE;
in vec3 fL;

out vec4 frag_color;

void main(void) {
  // Sample the texture to find the output color of this fragment
  frag_color = vec4(color, 1);
}
