#version 430

// In order to write to a texture, we have to introduce it as image2D.
// local_size_x/y/z layout variables define the work group size.
// gl_GlobalInvocationID is a uvec3 variable giving the global ID of the thread,
// gl_LocalInvocationID is the local index within the work group, and
// gl_WorkGroupID is the work group's index
layout(local_size_x = 1, local_size_y = 16, local_size_z = 1) in;

#define VOXEL_TYPE_AIR 0
#define VOXEL_TYPE_GRASS 1
struct Voxel {
	uint type;
	float isoLevel;
};

layout(std430, binding = 1) buffer bufferLayout
{ Voxel voxels[17][256][17]; };

uniform int chunkX;
uniform int chunkZ;

// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

/* skew constants for 3d simplex functions */
const float F3 =  0.3333333;
const float G3 =  0.1666667;

/* 3d simplex noise */
float simplex(vec3 p) {
	 /* 1. find current tetrahedron T and it's four vertices */
	 /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	 /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/

	 /* calculate s and x */
	 vec3 s = floor(p + dot(p, vec3(F3)));
	 vec3 x = p - s + dot(s, vec3(G3));

	 /* calculate i1 and i2 */
	 vec3 e = step(vec3(0.0), x - x.yzx);
	 vec3 i1 = e*(1.0 - e.zxy);
	 vec3 i2 = 1.0 - e.zxy*(1.0 - e);

	 /* x1, x2, x3 */
	 vec3 x1 = x - i1 + G3;
	 vec3 x2 = x - i2 + 2.0*G3;
	 vec3 x3 = x - 1.0 + 3.0*G3;

	 /* 2. find four surflets and store them in d */
	 vec4 w, d;

	 /* calculate surflet weights */
	 w.x = dot(x, x);
	 w.y = dot(x1, x1);
	 w.z = dot(x2, x2);
	 w.w = dot(x3, x3);

	 /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	 w = max(0.6 - w, 0.0);

	 /* calculate surflet components */
	 d.x = dot(random3(s), x);
	 d.y = dot(random3(s + i1), x1);
	 d.z = dot(random3(s + i2), x2);
	 d.w = dot(random3(s + 1.0), x3);

	 /* multiply d by w^4 */
	 w *= w;
	 w *= w;
	 d *= w;

	 /* 3. return the sum of the four surflets */
	 return dot(d, vec4(52.0));
}


void main() {
	int x = int(gl_GlobalInvocationID.x), y = int(gl_GlobalInvocationID.y), z = int(gl_GlobalInvocationID.z);
	int X = chunkX;
	int Z = chunkZ;

	float heightMap = (simplex(vec3((x + 16 * chunkX) / 100.0, (z + 16 * chunkZ) / 100.0, 0)) + 1) * 10 + 128;

	float factor;
	if(y < heightMap) factor = smoothstep(heightMap - 50, heightMap, vec2(y)).x;
	else factor = 1 - smoothstep(heightMap, heightMap + 5, vec2(y)).x;

	float noiseFunction = simplex(vec3((x + 16 * chunkX) / 20.0, y / 20.0, (z + 16 * chunkZ) / 20.0)) * 5;

	float function = factor * noiseFunction + (1 - factor) * (y - heightMap);

	voxels[x][y][z].isoLevel = function;
	if(function > 0)
		voxels[x][y][z].type = VOXEL_TYPE_AIR;
	else voxels[x][y][z].type = VOXEL_TYPE_GRASS;
}
