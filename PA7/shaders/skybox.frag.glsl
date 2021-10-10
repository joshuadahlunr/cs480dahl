#version 330
// Texture coordinates we map to (vec3 since using a cubemap)
in vec3 textureCoords;

// The texture we are sampling
uniform samplerCube skyboxTexture;

// Output color
out vec4 frag_color;

void main() {
  frag_color = texture(skyboxTexture, textureCoords);
}
