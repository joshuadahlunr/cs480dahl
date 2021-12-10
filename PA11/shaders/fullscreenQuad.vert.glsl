#version 330

// layout(location = 0) in vec2 v_position;

out vec2 UV;

vec2[6] verts = vec2[6](
   vec2(-1, -1),
	vec2(1, -1),
	vec2(-1, 1),

	vec2(1, 1),
	vec2(-1, 1),
	vec2(1, -1)
);

void main() {
   gl_Position = vec4(verts[gl_VertexID].xy, 0.0, 1.0);
   UV = 0.5 * gl_Position.xy + vec2(0.5);
}