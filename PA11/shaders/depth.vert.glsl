#version 330 core
layout (location = 0) in vec3 v_position;

uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

void main() {
	gl_Position = lightSpaceMatrix * modelMatrix * vec4(v_position, 1.0);
}