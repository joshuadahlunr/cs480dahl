#version 330

#define VOXEL_TYPE_AIR 0
#define VOXEL_TYPE_GRASS 1
#define VOXEL_TYPE_STONE 2

// ins
in vec3 lightingColor;
flat in vec3 varyingColor;
in vec2 varyingUV;

// uniforms
uniform sampler2D sampler;

// outs
out vec4 fragColor;

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
}