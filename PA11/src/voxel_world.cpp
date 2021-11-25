#include "voxel_world.h"

// Outer = X, Inner = Z

void VoxelWorld::initialize(glm::ivec2 playerChunk /*= {0, 0}*/){
	this->playerChunk = playerChunk;
	meshingQueue.getCompare().playerChunk = &this->playerChunk;

	for(int z = playerChunk.y - WORLD_RADIUS; z <= playerChunk.y + WORLD_RADIUS; z++){
		auto chunks = generateChunksZ(args, playerChunk.x - WORLD_RADIUS, z);
		for(int x = 0; x < chunks.size() - 1; x++){
			auto& chunk = chunks[x];
			chunk->setPosition({16 * (x + playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (z + playerChunk.y)});
			meshingQueue.push(chunk);
		}

		AddPosZ(chunks);
	}
}

void VoxelWorld::update(float dt){
    for(auto& row: chunks)
        for(auto& chunk: row)
            chunk->update(dt);

	// If there are chunks which need meshes generated for them... generate a mesh for those chunks
	if(!meshingQueue.empty()){
		auto nextMesh = meshingQueue.top();
		meshingQueue.pop();
		nextMesh->rebuildMesh(args);
		nextMesh->loadTextureFile(args, args.getResourcePath() + "textures/invalid.png");
	}
}

void VoxelWorld::render(Shader* boundShader){
    for(auto& row: chunks){
        for(auto& chunk: row){
            chunk->render(boundShader);
        }
    }
}


void VoxelWorld::stepPlayerPosX(){
	playerChunk.x++;

	auto chunks = generateChunksX(args, playerChunk.x + WORLD_RADIUS, playerChunk.y - WORLD_RADIUS);
	for(int z = 0; z < chunks.size() - 1; z++){
		auto& chunk = chunks[z];
		chunk->setPosition({16 * (playerChunk.x + WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (z + playerChunk.y - WORLD_RADIUS)});
		meshingQueue.push(chunk);
	}

	AddPosX(chunks);
}

void VoxelWorld::stepPlayerNegX(){
	playerChunk.x--;

	auto chunks = generateChunksX(args, playerChunk.x - WORLD_RADIUS, playerChunk.y - WORLD_RADIUS);
	for(int z = 0; z < chunks.size() - 1; z++){
		auto& chunk = chunks[z];
		chunk->setPosition({16 * (playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (z + playerChunk.y - WORLD_RADIUS)});
		meshingQueue.push(chunk);
	}

	AddNegX(chunks);
}

void VoxelWorld::stepPlayerPosZ(){
	playerChunk.y++;

	auto chunks = generateChunksZ(args, playerChunk.x - WORLD_RADIUS, playerChunk.y + WORLD_RADIUS);
	for(int x = 0; x < chunks.size() - 1; x++){
		auto& chunk = chunks[x];
		chunk->setPosition({16 * (x + playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (playerChunk.y + WORLD_RADIUS)});
		meshingQueue.push(chunk);
	}

	AddPosZ(chunks);
}

void VoxelWorld::stepPlayerNegZ(){
	playerChunk.y--;

	auto chunks = generateChunksZ(args, playerChunk.x - WORLD_RADIUS, playerChunk.y - WORLD_RADIUS);
	for(int x = 0; x < chunks.size() - 1; x++){
		auto& chunk = chunks[x];
		chunk->setPosition({16 * (x + playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (playerChunk.y - WORLD_RADIUS)});
		meshingQueue.push(chunk);
	}

	AddNegZ(chunks);
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
