#include "voxel_world.h"

// Outer = X, Inner = Z

void VoxelWorld::update(float dt){
    for(auto& row: chunks)
        for(auto& chunk: row)
            chunk->update(dt);
}

void VoxelWorld::render(Shader* boundShader){
    for(auto& row: chunks){
        for(auto& chunk: row){
            chunk->render(boundShader);
        }
    }
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddPosX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks) {
    for(int i = 0; i < chunks.size(); i++)
        this->chunks[i].emplace_back(chunks[i]);
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddNegX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks) {
    for(int i = 0; i < chunks.size(); i++)
        this->chunks[i].emplace_front(chunks[i]);
}

std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> VoxelWorld::generateChunksX(const Arguments& args, size_t X, size_t startZ) {
    std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> out;
    for(int i = 0; i < WORLD_RADIUS * 2 + 1; i++){ // one less, we don't generate the null chunk
        out[i] = std::make_shared<Chunk>();
	    out[i]->generateVoxels(args, X, startZ + i);
    }

    return out;
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddPosZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks) {
    this->chunks.emplace_front(chunks);
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddNegZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks) {
    this->chunks.emplace_back(chunks);
}

std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> VoxelWorld::generateChunksZ(const Arguments& args, size_t startX, size_t Z) {
    std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> out;
    for(int i = WORLD_RADIUS * 2; 0 <= i ; i--){ // one less, we don't generate the null chunk
        out[i] = std::make_shared<Chunk>();
	    out[i]->generateVoxels(args, startX + i, Z);
    }

    return out;
}
