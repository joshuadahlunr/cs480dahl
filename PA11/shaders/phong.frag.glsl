#version 430

#define VOXEL_TYPE_AIR 0
#define VOXEL_TYPE_GRASS 1
#define VOXEL_TYPE_STONE 2

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
layout(binding = 0) uniform sampler2D sampler;
layout(binding = 1) uniform sampler2D shadowMap;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

// Fog variables
uniform vec3 playerPosition;
uniform float worldRadius;

// ins
flat in vec3 varyingColor;
in vec2 varyingUV;
in vec3 varyingN;
in vec3 varyingP;
in vec4 worldPosition;
in vec4 lightSpacePosition;
flat in mat4 mv_matrix;

out vec4 fragColor;

// Function which calculates if the current pixel should be in shadow or not
float shadowCalculations(float normalLightDot){
	// perform perspective divide (normalized to [0, 1])
	vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	// get depth of current fragment from light's perspective (clampped to a maximum value of 1)
	float currentDepth = projCoords.z;
	if(currentDepth > 1) currentDepth = 1;

	// Calculate the bias to reduce stair-stepping
	double bias = max(.01 * (1.0 - normalLightDot), .0005); // TODO: Tweak so that we can see the UFO's shadow when it lands

	// Sample several textures around the current texture and average the results
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
		for(int y = -1; y <= 1; ++y) {
			double pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    

	// Return the averaged result of the differet pixels
	return shadow / 9;
}

// Function which preforms lighting calculations
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
		L = -normalize(light.direction);
	} else if(light.type == TYPE_POINT || light.type == TYPE_SPOT){
		L = normalize(viewMatrix * light.position - P).xyz;

		if(light.type == TYPE_SPOT){
			float phi = dot(normalize(-light.direction), normalize((light.position - worldPosition).xyz));
			float intensityFactor = pow(phi, light.intensity);

			spotlightFalloff = smoothstep(light.cutoffAngleCosine, light.cutoffAngleCosine + light.falloff, intensityFactor);
		}
	}

	vec3 R = normalize(reflect(-L,N));

	float kD = max(dot(N,L), 0.0);
	vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * kD * spotlightFalloff;
	vec3 specular = material.specular.xyz * light.specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess) * spotlightFalloff;
	if(kD == 0) specular = vec3(0);

	float shadowMask = 0;
	if(light.type == TYPE_DIRECTIONAL) shadowMask = shadowCalculations(kD);

	return ambient + (1.0 - shadowMask) * (diffuse + specular);
} 

// Function which returns 0 when there shouldn't be fog, 1 when there should be and smoothly blends between them
float fogMask(){
	float distSq = dot(worldPosition.xyz - playerPosition, worldPosition.xyz - playerPosition);

	return smoothstep((worldRadius - 32) * (worldRadius - 32), worldRadius * worldRadius, distSq);
}

// Function which converts a voxel type to a color
vec3 typeToColor(int type){
	switch(type){
	case VOXEL_TYPE_GRASS: return vec3(.1, 1, .1);
	case VOXEL_TYPE_STONE: return vec3(.7, .7, .7);
	default: return vec3(1);
	}
}

void main(void) {
	vec3 N = normalize(varyingN);
	vec3 V = normalize(-varyingP);

	vec3 color = vec3(0);
	for(uint i = 0u; i < num_lights; i++)
	   color += calculateLighting(lights[i], viewMatrix * worldPosition, N, V);

	if(varyingColor.x > 0) color *= texture(sampler, varyingUV).rgb;
	fragColor = vec4(color * typeToColor( int(varyingColor.y)), 1);

	/// Apply fog
	float mask = fogMask();
	fragColor = mask * vec4(.7, .7, .7, 1) + (1 - mask) * fragColor;
}
