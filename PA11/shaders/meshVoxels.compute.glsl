#version 430

// In order to write to a texture, we have to introduce it as image2D.
// local_size_x/y/z layout variables define the work group size.
// gl_GlobalInvocationID is a uvec3 variable giving the global ID of the thread,
// gl_LocalInvocationID is the local index within the work group, and
// gl_WorkGroupID is the work group's index
layout(local_size_x = 1, local_size_y = 16, local_size_z = 1) in;

struct Voxel {
	uint type;
	float isoLevel;
};

struct Triangle {
	vec4 points[3];
};

// layout (binding = 1, offset = 0) uniform atomic_uint verteciesUsed;
// layout (binding = 1, offset = 4) uniform atomic_uint indeciesUsed;

layout(std430, binding = 1) buffer inBuffer
{ Voxel voxels[17][256][17]; };

layout(std430, binding = 2) coherent buffer vertexBuffer {
	uint verteciesUsed;
	Triangle vertecies[1024 * 3]; // No vec3s in buffers
};
layout(std430, binding = 3) coherent buffer indexBuffer {
	uint indeciesUsed;
	uint indecies[1024 * 3];
};

// Get the lookup tables from cpu
layout(std430, binding = 4) buffer lookupBuffer {
	int edgeTable[256];
	int triTable[256][16];
};


// Structure holding 8 voxel samples we create our mesh from
struct IsoGridSample {
	vec3 points[8];
	float values[8];
};

/*
	Linearly interpolate the position where an isosurface cuts
	an edge between two vertices, each with their own scalar value
*/
vec3 vertexInterp(float isolevel, vec3 p1, vec3 p2, float valp1, float valp2) {
	float mu;
	vec3 p;

	if (abs(isolevel-valp1) < 0.00001)
		return(p1);
	if (abs(isolevel-valp2) < 0.00001)
		return(p2);
	if (abs(valp1-valp2) < 0.00001)
		return(p1);

	mu = (isolevel - valp1) / (valp2 - valp1);
	p.x = p1.x + mu * (p2.x - p1.x);
	p.y = p1.y + mu * (p2.y - p1.y);
	p.z = p1.z + mu * (p2.z - p1.z);

	return p;
}

// Calculates a marching cubes approximation of a single grid sample of a voxelized IsoFunction with a surface at <isoLevel>
// Implementation from: https://paulbourke.net/geometry/polygonise/
int calculateMarchingCubes(IsoGridSample grid, out Triangle triangles[5], float isolevel) {
	int i, ntriang;
	vec3 vertlist[12];

	/*
		Determine the index into the edge table which
		tells us which vertices are inside of the surface
	*/
	int cubeindex = 0;
	if (grid.values[0] < isolevel) cubeindex |= 1;
	if (grid.values[1] < isolevel) cubeindex |= 2;
	if (grid.values[2] < isolevel) cubeindex |= 4;
	if (grid.values[3] < isolevel) cubeindex |= 8;
	if (grid.values[4] < isolevel) cubeindex |= 16;
	if (grid.values[5] < isolevel) cubeindex |= 32;
	if (grid.values[6] < isolevel) cubeindex |= 64;
	if (grid.values[7] < isolevel) cubeindex |= 128;

	/* Cube is entirely in/out of the surface */
	if (edgeTable[cubeindex] == 0)
		return 0;

	/* Find the vertices where the surface intersects the cube */
	if ((edgeTable[cubeindex] & 1) > 0)
		vertlist[0] = vertexInterp(isolevel, grid.points[0], grid.points[1], grid.values[0], grid.values[1]);
	if ((edgeTable[cubeindex] & 2) > 0)
		vertlist[1] = vertexInterp(isolevel, grid.points[1], grid.points[2], grid.values[1], grid.values[2]);
	if ((edgeTable[cubeindex] & 4) > 0)
		vertlist[2] = vertexInterp(isolevel, grid.points[2], grid.points[3], grid.values[2], grid.values[3]);
	if ((edgeTable[cubeindex] & 8) > 0)
		vertlist[3] = vertexInterp(isolevel, grid.points[3], grid.points[0], grid.values[3], grid.values[0]);
	if ((edgeTable[cubeindex] & 16) > 0)
		vertlist[4] = vertexInterp(isolevel, grid.points[4], grid.points[5], grid.values[4], grid.values[5]);
	if ((edgeTable[cubeindex] & 32) > 0)
		vertlist[5] = vertexInterp(isolevel, grid.points[5], grid.points[6], grid.values[5], grid.values[6]);
	if ((edgeTable[cubeindex] & 64) > 0)
		vertlist[6] = vertexInterp(isolevel, grid.points[6], grid.points[7], grid.values[6], grid.values[7]);
	if ((edgeTable[cubeindex] & 128) > 0)
		vertlist[7] = vertexInterp(isolevel, grid.points[7], grid.points[4], grid.values[7], grid.values[4]);
	if ((edgeTable[cubeindex] & 256) > 0)
		vertlist[8] = vertexInterp(isolevel, grid.points[0], grid.points[4], grid.values[0], grid.values[4]);
	if ((edgeTable[cubeindex] & 512) > 0)
		vertlist[9] = vertexInterp(isolevel, grid.points[1], grid.points[5], grid.values[1], grid.values[5]);
	if ((edgeTable[cubeindex] & 1024) > 0)
		vertlist[10] = vertexInterp(isolevel, grid.points[2], grid.points[6], grid.values[2], grid.values[6]);
	if ((edgeTable[cubeindex] & 2048) > 0)
		vertlist[11] = vertexInterp(isolevel, grid.points[3], grid.points[7], grid.values[3], grid.values[7]);

	/* Create the triangle */
	ntriang = 0;
	for (i = 0; triTable[cubeindex][i] != -1; i += 3) {
		triangles[ntriang].points[0] = vec4(vertlist[triTable[cubeindex][i]], 0);
		triangles[ntriang].points[1] = vec4(vertlist[triTable[cubeindex][i+1]], 0);
		triangles[ntriang].points[2] = vec4(vertlist[triTable[cubeindex][i+2]], 0);
		ntriang++;
	}

	// memoryBarrierShared();
	// enablePrintf();
	// printf("%g, ", ntriang);
	// disablePrintf();

	return ntriang;
}

void main() {
	int x = int(gl_GlobalInvocationID.x), y = int(gl_GlobalInvocationID.y), z = int(gl_GlobalInvocationID.z);

	// Define our sample
	IsoGridSample cell;
	float scale = 1; // TODO: calculate from chunk width
	cell.points[0] = vec3(x, y, z);
	cell.values[0] = voxels[x][y][z].isoLevel;
	cell.points[1] = vec3(x + scale, y, z);
	cell.values[1] = voxels[x + 1][y][z].isoLevel;
	cell.points[2] = vec3(x + scale, y, z + scale);
	cell.values[2] = voxels[x + 1][y][z + 1].isoLevel;
	cell.points[3] = vec3(x, y, z + scale);
	cell.values[3] = voxels[x][y][z + 1].isoLevel;
	cell.points[4] = vec3(x, y + scale, z);
	cell.values[4] = voxels[x][y + 1][z].isoLevel;
	cell.points[5] = vec3(x + scale, y + scale, z);
	cell.values[5] = voxels[x + 1][y + 1][z].isoLevel;
	cell.points[6] = vec3(x + scale, y + scale, z + scale);
	cell.values[6] = voxels[x + 1][y + 1][z + 1].isoLevel;
	cell.points[7] = vec3(x, y + scale, z + scale);
	cell.values[7] = voxels[x][y + 1][z + 1].isoLevel;

	Triangle triangles[5];
	int usedTriangles = calculateMarchingCubes(cell, triangles, 0);

	//if(gl_WorkGroupID == ivec3(0)) enablePrintf();
	enablePrintf();
	barrier();

	for(int t = 0; t < usedTriangles; t++){
		// Wait for everyone else to stop writing before we add our vertecies
		memoryBarrierShared();
		// printf("%g - %g, ", verteciesUsed, indeciesUsed);

		vertecies[verteciesUsed] = triangles[t];
		// indecies[indeciesUsed] = indeciesUsed;
		// indecies[indeciesUsed + 1] = indeciesUsed + 1;
		// indecies[indeciesUsed + 2] = indeciesUsed + 2;

		// atomicCounterIncrement(indeciesUsed);
		// atomicCounterIncrement(verteciesUsed);
		// atomicAdd(indeciesUsed, 3);
		atomicAdd(verteciesUsed, 1);
	}

	// if(y == 0) enablePrintf();
	// printf("%g - %^3g - %g\n", usedTriangles, cell.points[1], cell.values[1]);

	//memoryBarrierShared();
	//verteciesUsed = int(cell.points[1].x);
	//indeciesUsed = int(cell.values[1]);
}
