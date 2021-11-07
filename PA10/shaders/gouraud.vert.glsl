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

void main(void) { 
    mat4 mv_matrix = viewMatrix * modelMatrix;
    mat4 norm_matrix = transpose(inverse(mv_matrix));

    vec4 P = mv_matrix * vec4(v_position,1.0);
    vec3 N = normalize((norm_matrix * vec4(v_normal,1.0)).xyz);
    vec3 L = normalize((mv_matrix * spotlights[0].position).xyz - P.xyz);
    vec3 V = normalize(-P.xyz);
    vec3 R = reflect(-L,N);

    float phi = dot(-normalize(spotlights[0].direction), normalize((spotlights[0].position - modelMatrix * vec4(v_position, 1)).xyz));
    float intensityFactor = pow(phi, spotlights[0].intensity);
    float falloff = smoothstep(spotlights[0].cutoffAngleCosine, spotlights[0].cutoffAngleCosine + spotlights[0].falloff, intensityFactor);

    vec3 ambient = ((globallight.ambient * material.ambient) + (spotlights[0].ambient * material.ambient)).xyz;
   
    vec3 diffuse = spotlights[0].diffuse.xyz * material.diffuse.xyz * max(dot(N,L), 0.0) * falloff;
    vec3 specular = material.specular.xyz * spotlights[0].specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess) * falloff;

    varyingColor = ambient + diffuse + specular;

    varyingUV = v_uv;

    gl_Position = projectionMatrix * P;
}