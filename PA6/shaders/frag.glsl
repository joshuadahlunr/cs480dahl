#version 330

smooth in vec3 color;
smooth in vec2 uv;
uniform sampler2D sampler;

out vec4 frag_color;

void main(void) {
  frag_color = texture2D(sampler, uv);
}
