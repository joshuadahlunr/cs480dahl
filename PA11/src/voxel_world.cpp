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
        this->chunks[i].push_front(chunks[i]);
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddNegX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks) {
    for(int i = 0; i < chunks.size(); i++)
        this->chunks[i].push_back(chunks[i]);
}

std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> VoxelWorld::generateChunksX(size_t X, size_t startZ) {
    std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> out;
    for(int i = 0; i < WORLD_RADIUS * 2 + 1; i++){ // one less, we don't generate the null chunk 
        out[i] = std::make_shared<Chunk>();
	    out[i]->generateVoxels(X, startZ + i);
    }

    return out;
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddPosZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks) {
    rowBuffer b = chunks;
    this->chunks.push_front(b);
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddNegZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2>& chunks) {
    this->chunks.push_back(rowBuffer(chunks));
}

std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> VoxelWorld::generateChunksZ(size_t startX, size_t Z) {
    std::array<Chunk::ptr, WORLD_RADIUS * 2 + 2> out;
    for(int i = 0; i < WORLD_RADIUS * 2 + 1; i++){ // one less, we don't generate the null chunk 
        out[i] = std::make_shared<Chunk>();
	    out[i]->generateVoxels(startX + i, Z);
    }

    return out;
}