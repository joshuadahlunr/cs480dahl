#version 330

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

// ins
in vec3 varyingN;
in vec3 varyingL;
in vec3 varyingP;
in vec4 rawPosition;
in mat4 mv_matrix;

out vec4 fragColor;

void main(void)
{  
    vec3 L = normalize(varyingL);
    vec3 N = normalize(varyingN);
    vec3 V = normalize(-varyingP);

    vec3 R = normalize(reflect(-L, N));

    float phi = dot(-normalize(spotlights[0].direction), normalize((spotlights[0].position - modelMatrix * rawPosition).xyz));
    //float phi = dot(-normalize(spotlights[0].direction), L);
    float intensityFactor = pow(phi, spotlights[0].intensity);

    vec3 ambient = ((globallight.ambient * material.ambient) + (spotlights[0].ambient * material.ambient)).xyz;
    float spotlightFalloff = smoothstep(spotlights[0].cutoffAngleCosine, spotlights[0].cutoffAngleCosine + spotlights[0].falloff, intensityFactor);
   
    vec3 diffuse = spotlights[0].diffuse.xyz * material.diffuse.xyz * max(dot(N,L), 0.0) * spotlightFalloff;
    vec3 specular = material.specular.xyz * spotlights[0].specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess) * spotlightFalloff;

    fragColor = vec4(ambient + diffuse + specular, 1);
}