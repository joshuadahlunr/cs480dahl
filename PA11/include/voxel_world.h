#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include "chunk.h"
#include "circular_buffer.hpp"

#define WORLD_RADIUS 10

struct VoxelWorld {
    // Outer = X, Inner = Z
    using rowBuffer = circular_buffer<Chunk::ptr, std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>>;
    circular_buffer<rowBuffer, std::array<rowBuffer, WORLD_RADIUS * 2 + 2>> chunks;

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
