#version 330

// attributes
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_uv;
layout (location = 3) in vec3 v_normal;

// structs
struct GlobalLight
{ 
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
    vec3 direction;
};

struct PointLight
{ 
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
};

struct SpotLight
{
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
#define MAX_POINTLIGHTS 20
#define MAX_SPOTLIGHTS 10

// uniforms
uniform GlobalLight globallight;
uniform int num_pointlights;
uniform PointLight pointlights[MAX_POINTLIGHTS];
uniform int num_spotlights;
uniform SpotLight spotlights[MAX_SPOTLIGHTS];

uniform Material material;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// outs
out vec3 varyingColor;
out vec2 varyingUV;
out vec3 varyingP;
out vec3 varyingL;
out vec3 varyingN;
out mat4 mv_matrix;
out vec4 rawPosition;

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    return vec3(0,0,0);
}

void main(void)
{  
    mat4 mv_matrix = viewMatrix * modelMatrix;
    mat4 norm_matrix = transpose(inverse(mv_matrix));

    // output vertex position, light direction, and normal to the rasterizer for interpolation
    varyingP = (mv_matrix * vec4(v_position,1.0)).xyz;
    //varyingL = light.position.xyz - varyingP;
    varyingL = (mv_matrix * spotlights[0].position).xyz - varyingP;
    varyingN = (norm_matrix * vec4(v_normal,1.0)).xyz;

    varyingUV = v_uv;

    gl_Position = projectionMatrix * mv_matrix * vec4(v_position,1.0);
    rawPosition = vec4(v_position, 1);
}