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

#define NOISE_SEED 12345

//	Simplex 4D Noise 
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
float permute(float x){return floor(mod(((x*34.0)+1.0)*x, 289.0));}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
float taylorInvSqrt(float r){return 1.79284291400159 - 0.85373472095314 * r;}

vec4 grad4(float j, vec4 ip){
  const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
  vec4 p,s;

  p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
  p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
  s = vec4(lessThan(p, vec4(0.0)));
  p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

  return p;
}

float simplex(vec4 v){
  const vec2  C = vec2( 0.138196601125010504,  // (5 - sqrt(5))/20  G4
                        0.309016994374947451); // (sqrt(5) - 1)/4   F4
// First corner
  vec4 i  = floor(v + dot(v, C.yyyy) );
  vec4 x0 = v -   i + dot(i, C.xxxx);

// Other corners

// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
  vec4 i0;

  vec3 isX = step( x0.yzw, x0.xxx );
  vec3 isYZ = step( x0.zww, x0.yyz );
//  i0.x = dot( isX, vec3( 1.0 ) );
  i0.x = isX.x + isX.y + isX.z;
  i0.yzw = 1.0 - isX;

//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
  i0.y += isYZ.x + isYZ.y;
  i0.zw += 1.0 - isYZ.xy;

  i0.z += isYZ.z;
  i0.w += 1.0 - isYZ.z;

  // i0 now contains the unique values 0,1,2,3 in each channel
  vec4 i3 = clamp( i0, 0.0, 1.0 );
  vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
  vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

  //  x0 = x0 - 0.0 + 0.0 * C 
  vec4 x1 = x0 - i1 + 1.0 * C.xxxx;
  vec4 x2 = x0 - i2 + 2.0 * C.xxxx;
  vec4 x3 = x0 - i3 + 3.0 * C.xxxx;
  vec4 x4 = x0 - 1.0 + 4.0 * C.xxxx;

// Permutations
  i = mod(i, 289.0); 
  float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
  vec4 j1 = permute( permute( permute( permute (
             i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
           + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
           + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
           + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));
// Gradients
// ( 7*7*6 points uniformly over a cube, mapped onto a 4-octahedron.)
// 7*7*6 = 294, which is close to the ring size 17*17 = 289.

  vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

  vec4 p0 = grad4(j0,   ip);
  vec4 p1 = grad4(j1.x, ip);
  vec4 p2 = grad4(j1.y, ip);
  vec4 p3 = grad4(j1.z, ip);
  vec4 p4 = grad4(j1.w, ip);

// Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;
  p4 *= taylorInvSqrt(dot(p4,p4));

// Mix contributions from the five corners
  vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
  vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
  m0 = m0 * m0;
  m1 = m1 * m1;
  return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
               + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

}

// Lower dimensional simplex noise with seeding
float simplex(vec3 p, float seed) { return simplex(vec4(p, seed)); }
float simplex(vec2 p, float seed) { return simplex(vec4(p, seed, seed + 1)); }

void main() {
	int x = int(gl_GlobalInvocationID.x), y = int(gl_GlobalInvocationID.y), z = int(gl_GlobalInvocationID.z);
	int X = chunkX;
	int Z = chunkZ;

  int octaves = 3;
  float persistance = 0.2f;
  float lacunarity = 2.0f;
  float scale = 16 * 8;
  
  float amplitude = 28.0f;
  float frequency = 1.0f;
  float height = 0.0f;
  float xCoord = (x + 16 * chunkX) / scale;
  float yCoord = (z + 16 * chunkZ) / scale;

  float heightMap = 0.0f;

  for(int i = 0; i < octaves; i++) {
    float height = simplex(vec2(xCoord * frequency, yCoord * frequency), NOISE_SEED * i) + 1;
    heightMap += height * amplitude;
    amplitude *= persistance;
    frequency *= lacunarity;
  }


	// float heightMap = 0;
  // for(int i = 1; i <= 4; i++){
  //   heightMap += (simplex(vec2((x + 16 * chunkX) / 1000.0 * i, (z + 16 * chunkZ) / 1000.0 * i), NOISE_SEED * i) + 1) * 100 / i - 100 / i;
  // }


  heightMap = pow(abs(heightMap) / 30, 6);

  // float terrace = smoothTerrace(heightMap / 10) * 10;
  // float terrace = floor(heightMap / 10) * 10;
  // heightMap *= 2;
  // heightMap += terrace;
  // heightMap /= 3;

  // heightMap = clamp(heightMap + 20, -257, 257);

  
  

	// float factor;
	// if(y < heightMap) factor = smoothstep(heightMap - 50, heightMap, vec2(y)).x;
	// else factor = 1 - smoothstep(heightMap, heightMap + 5, vec2(y)).x;

	// float noiseFunction = simplex(vec3((x + 16 * chunkX) / 20.0, y / 20.0, (z + 16 * chunkZ) / 20.0), NOISE_SEED * 20) * 5;

	// float function = factor * noiseFunction + (1 - factor) * (y - heightMap);
  float function = y - heightMap;


	voxels[x][y][z].isoLevel = function;
	if(function > 0)
		voxels[x][y][z].type = VOXEL_TYPE_AIR;
	else voxels[x][y][z].type = VOXEL_TYPE_GRASS;
}
