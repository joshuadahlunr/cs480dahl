#version 330

#define VOXEL_TYPE_AIR 0
#define VOXEL_TYPE_GRASS 1
#define VOXEL_TYPE_STONE 2

// ins
in vec3 lightingColor;
flat in vec3 varyingColor;
in vec2 varyingUV;
in vec4 worldPosition;

// uniforms
uniform sampler2D sampler;

// Fog variables
uniform vec3 playerPosition;
uniform float worldRadius;


// outs
out vec4 fragColor;

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
	default: return vec3(0, 0, 0);
	}
}

void main(void) {
	fragColor = vec4(lightingColor * typeToColor(int(varyingColor.y)), 1);
	if(varyingColor.x > 0) fragColor *= texture2D(sampler, varyingUV);

	// Apply fog
	float mask = fogMask();
	fragColor = mask * vec4(.7, .7, .7, 1) + (1 - mask) * fragColor;
}