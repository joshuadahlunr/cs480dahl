#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include "chunk.h"
#include "circular_buffer.hpp"

#define WORLD_RADIUS 10

struct VoxelWorld {
	// Circular Buffer of Circular Buffers of Chunks
	// Need +1 on the radius to include 0,0 and another +1 for the empty slot the buffer requires
    circular_buffer_array<circular_buffer_array<Chunk::ptr, WORLD_RADIUS * 2 + 2>, WORLD_RADIUS * 2 + 2> chunks; // Outer = X, Inner = Z

    void update(float dt);
    void render(Shader* boundShader);

    void AddPosX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks);
    void AddNegX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks);

    static std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> generateChunksX(const Arguments& args, size_t X, size_t startZ);

    void AddPosZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks);
    void AddNegZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks);

    static std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> generateChunksZ(const Arguments& args, size_t startX, size_t Z);
};

#endif // VOXEL_WORLD_H
