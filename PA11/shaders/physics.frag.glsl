#version 330

// Load colors and UVs from the vertex file
in vec3 color;

// We output a vec4 color
out vec4 frag_color;

void main(void) {
	// Sample the texture to find the output color of this fragment
	frag_color = vec4(color, 1);
}
