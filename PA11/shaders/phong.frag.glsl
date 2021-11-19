#version 330

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

// ins
in vec3 varyingColor;
in vec2 varyingUV;
in vec3 varyingN;
in vec3 varyingP;
in vec4 rawPosition;
flat in mat4 mv_matrix;

out vec4 fragColor;

vec3 calculateLighting(Light light, vec4 P, vec3 N, vec3 V){
	// Disabled lights contribute nothing
	if(light.type == TYPE_DISABLED)
		return vec3(0);

	vec3 ambient = (light.ambient * material.ambient).xyz;
	vec3 L = vec3(0);
    float spotlightFalloff = 1;

    if(light.type == TYPE_AMBIENT){
        return ambient;
    } else if (light.type == TYPE_DIRECTIONAL){
        L = normalize(light.direction);
    } else if(light.type == TYPE_POINT || light.type == TYPE_SPOT){
        L = normalize(viewMatrix * light.position - P).xyz;

        if(light.type == TYPE_SPOT){
            float phi = dot(normalize(-light.direction), normalize((light.position - modelMatrix * rawPosition).xyz));
            float intensityFactor = pow(phi, light.intensity);

            spotlightFalloff = smoothstep(light.cutoffAngleCosine, light.cutoffAngleCosine + light.falloff, intensityFactor);
        }
    }

    vec3 R = normalize(reflect(-L,N));

    vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(dot(N,L), 0.0) * spotlightFalloff;
    vec3 specular = material.specular.xyz * light.specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess) * spotlightFalloff;
	if(max(dot(N,L), 0.0) == 0) specular = vec3(0);

    return ambient + diffuse + specular;
}

void main(void) {
    vec3 N = normalize(varyingN);
    vec3 V = normalize(-varyingP);

    vec3 color = vec3(0);
    for(uint i = 0u; i < num_lights; i++)
       color += calculateLighting(lights[i], mv_matrix * rawPosition, N, V);

    color *= texture2D(sampler, varyingUV).rgb;
    fragColor = vec4(color, 1);
}
