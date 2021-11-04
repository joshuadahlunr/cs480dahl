#version 330

// structs
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

// uniforms
uniform vec4 globalAmbient;

uniform SpotLight light;
uniform Material material;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// ins
in vec3 varyingN;
in vec3 varyingL;
in vec3 varyingP;
in vec4 rawPosition;

out vec4 fragColor;
// structs and uniforms same as for Gouraud shading

void main(void)
{  
    vec3 L = normalize(varyingL);
    vec3 N = normalize(varyingN);
    vec3 V = normalize(-varyingP);

    vec3 R = normalize(reflect(-L, N));

    float phi = dot(-normalize(light.direction), normalize((light.position - modelMatrix * rawPosition).xyz));
    //float phi = dot(-normalize(light.direction), L);
    float intensityFactor = pow(phi, light.intensity);

    vec3 ambient = ((globalAmbient * material.ambient) + (light.ambient * material.ambient)).xyz;
    float spotlightFalloff = smoothstep(light.cutoffAngleCosine, light.cutoffAngleCosine + light.falloff, intensityFactor);
   
    vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(dot(N,L), 0.0) * spotlightFalloff;
    vec3 specular = material.specular.xyz * light.specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess) * spotlightFalloff;

    fragColor = vec4(ambient + diffuse + specular, 1);


    //fragColor = vec4(1, 0, 0, 1);
}