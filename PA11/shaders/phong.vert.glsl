#version 330

// attributes
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_uv;
layout (location = 3) in vec3 v_normal;

// structs
#define TYPE_DISABLED 0u
#define TYPE_AMBIENT 1u
#define TYPE_DIRECTIONAL 2u
#define TYPE_POINT 3u
#define TYPE_SPOT 4u

struct Light
{
    uint type;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
    vec3 direction;
    float cutoffAngleCosine;
    float intensity;
    float falloff;
};

struct Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

// defines
#define MAX_LIGHTS 20

// uniforms
uniform Light lights[MAX_LIGHTS];
uniform uint num_lights;

uniform Material material;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// outs
flat out vec3 varyingColor;
out vec2 varyingUV;
out vec3 varyingP;
out vec3 varyingN;
out vec4 rawPosition;
flat out mat4 mv_matrix;

void main(void) {
    mv_matrix = viewMatrix * modelMatrix;
    mat4 norm_matrix = transpose(inverse(mv_matrix));

    // output vertex position and normal to the rasterizer for interpolation
    varyingP = (mv_matrix * vec4(v_position,1.0)).xyz;
    varyingN = (norm_matrix * vec4(v_normal,1.0)).xyz;

    gl_Position = projectionMatrix * mv_matrix * vec4(v_position,1.0);
    rawPosition = vec4(v_position, 1);

	varyingColor = v_color;
	varyingUV = v_uv;
}
