#version 330

// Load the position, color, and texture coordinates from the CPU
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
// Load the MVP matricies from the CPU
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

// We output color and textures coordinates
out vec3 color;

void main(void) {
	// Make the position we loaded homogeneous and apply MVP
	gl_Position = projectionMatrix * viewMatrix * vec4(v_position, 1.0);
	// Output the color and texture coordinates
	color = v_color;
}
