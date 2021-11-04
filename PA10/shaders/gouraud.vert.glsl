#version 330

// attributes
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 v_uv;
layout (location = 3) in vec3 v_normal;

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

// outs
out vec3 varyingColor;
out vec2 varyingUV;

void main(void) { 
    mat4 mv_matrix = viewMatrix * modelMatrix;
    mat4 norm_matrix = transpose(inverse(mv_matrix));

    vec4 P = mv_matrix * vec4(v_position,1.0);
    vec3 N = normalize((norm_matrix * vec4(v_normal,1.0)).xyz);
    vec3 L = normalize((mv_matrix * light.position).xyz - P.xyz);
    vec3 V = normalize(-P.xyz);
    vec3 R = reflect(-L,N);

    float phi = dot(-normalize(light.direction), normalize((light.position - modelMatrix * vec4(v_position, 1)).xyz));
    float intensityFactor = pow(phi, light.intensity);
    float spotlightFalloff = smoothstep(light.cutoffAngleCosine, light.cutoffAngleCosine + light.falloff, intensityFactor);

    vec3 ambient = ((globalAmbient * material.ambient) + (light.ambient * material.ambient)).xyz;
   
    vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(dot(N,L), 0.0) * spotlightFalloff;
    vec3 specular = material.specular.xyz * light.specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess) * spotlightFalloff;

    varyingColor = ambient + diffuse + specular;

    varyingUV = v_uv;

    gl_Position = projectionMatrix * P;
}