#version 330

// Load colors and UVs from the vertex file
smooth in vec3 color;
smooth in vec2 uv;
// Create a texture sampler linked to texture 0
uniform sampler2D sampler;

// We output a vec4 color
out vec4 frag_color;

void main(void) {
  // Sample the texture to find the output color of this fragment
  frag_color = texture2D(sampler, uv);
}
