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
uniform sampler2D sampler;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// outs
out vec3 lightingColor;
flat out vec3 varyingColor;
out vec2 varyingUV;

vec3 calculateLighting(Light light, vec4 P, vec3 N, vec3 V, mat4 mv_matrix){
	// Disabled lights contribute nothing
	if(light.type == TYPE_DISABLED)
		return vec3(0);

    vec3 ambient = (light.ambient * material.ambient).xyz;
    vec3 L = vec3(0);
    float falloff = 1;

    if(light.type == TYPE_AMBIENT){
        return ambient;
    } else if (light.type == TYPE_DIRECTIONAL){
        L = normalize(light.direction);
    } else if(light.type == TYPE_POINT || light.type == TYPE_SPOT){
        L = normalize((mv_matrix * light.position).xyz - P.xyz);

        if(light.type == TYPE_SPOT){
            float phi = dot(-normalize(light.direction), normalize((light.position - modelMatrix * vec4(v_position, 1)).xyz));
            float intensityFactor = pow(phi, light.intensity);
            falloff = smoothstep(light.cutoffAngleCosine, light.cutoffAngleCosine + light.falloff, intensityFactor);
        }
    }

    vec3 R = normalize(reflect(-L,N));

    vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(dot(N,L), 0.0) * falloff;
    vec3 specular = material.specular.xyz * light.specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess) * falloff;
	if(max(dot(N,L), 0.0) == 0) specular = vec3(0);

    return ambient + diffuse + specular;
}

void main(void) {
    mat4 mv_matrix = viewMatrix * modelMatrix;
    mat4 norm_matrix = transpose(inverse(mv_matrix));

    vec4 P = mv_matrix * vec4(v_position,1.0);
    vec3 N = normalize((norm_matrix * vec4(v_normal,1.0)).xyz);
    vec3 V = normalize(-P.xyz);

    lightingColor = vec3(0);
    for(uint i = 0u; i < num_lights; i++)
       lightingColor += calculateLighting(lights[i], P, N, V, mv_matrix);

    gl_Position = projectionMatrix * P;
    varyingColor = v_color;
	varyingUV = v_uv;
}
