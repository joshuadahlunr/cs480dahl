#include "voxel_world.h"

// Memory backing for the generation queue
ModifiablePriorityQueue<std::pair<Chunk::ptr, glm::ivec2>, std::vector<std::pair<Chunk::ptr, glm::ivec2>>, VoxelWorld::MeshingSort> VoxelWorld::generationQueue;

// Outer = X, Inner = Z

void VoxelWorld::initialize(glm::ivec2 playerChunk /*= {0, 0}*/){
	this->playerChunk = playerChunk;
	generationQueue.getCompare().playerChunk = &this->playerChunk;
	meshingQueue->getCompare().playerChunk = &this->playerChunk;

	for(int z = playerChunk.y - WORLD_RADIUS; z <= playerChunk.y + WORLD_RADIUS; z++){
		auto chunks = generateChunksZ(args, playerChunk.x - WORLD_RADIUS, z);
		for(int x = 0; x < chunks.size(); x++){
			auto& chunk = chunks[x];
			chunk->setPosition({16 * (x + playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (z + playerChunk.y)});
		}

		AddPosZ(chunks);
	}


	// Stop the meshing thread of already started
	if(meshingThread.joinable()){
		shouldMeshingThreadRun = false;
		meshingThread.join();
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	// Start a new meshing thread which just meshes chunks while there are chunks to be meshed
	meshingThread = std::thread([this](){
		while(shouldMeshingThreadRun){
			// If there are chunks which need meshes generated for them... generate a mesh for those chunks
			if(!meshingQueue.unsafe().empty()){
				auto nextMesh = meshingQueue.read_lock()->top();
				meshingQueue->pop();
				nextMesh->rebuildMesh(args);
				nextMesh->state = Chunk::GenerateState::Meshed;

				// Upload the meshed data to the gpu
				uploadQueue.push(nextMesh);

			// If there aren't chunks to generate... sleep for 5 milliseconds
			} else std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	});
}

void VoxelWorld::update(float dt){
    for(auto& row: chunks)
        for(auto& chunk: row)
            chunk->update(dt);

	// If there are chunks which need their data generated... generate one
	if(!generationQueue.empty()){
		auto& nextGeneration = generationQueue.top();
		generationQueue.pop();
		nextGeneration.first->generateVoxels(args, nextGeneration.second.x, nextGeneration.second.y);
		nextGeneration.first->state = Chunk::GenerateState::Generated;

		// Add this chunk to the meshing queue
		meshingQueue->push(nextGeneration.first);
	}

	// If there are meshed chunks which need to be uploaded to the gpu... upload them
	while(!uploadQueue.empty()){
		auto nextMesh = uploadQueue.front();
		uploadQueue.pop();

		// Upload the model to the gpu
		nextMesh->finalizeModel(); // TODO: Do we need to clear the current model?
		nextMesh->loadTextureFile(args, args.getResourcePath() + "textures/invalid.png");
		nextMesh->state = Chunk::GenerateState::Finalized;
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
	auto chunks = generateChunksX(args, playerChunk.x + WORLD_RADIUS, playerChunk.y - WORLD_RADIUS);
	for(int z = 0; z < chunks.size(); z++){
		auto& chunk = chunks[z];
		chunk->setPosition({16 * (playerChunk.x + WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (z + playerChunk.y - WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddPosX(chunks);
	playerChunk.x++;
}

void VoxelWorld::stepPlayerNegX(){
	playerChunk.x--;

	auto chunks = generateChunksX(args, playerChunk.x - WORLD_RADIUS, playerChunk.y - WORLD_RADIUS);
	for(int z = 0; z < chunks.size(); z++){
		auto& chunk = chunks[z];
		chunk->setPosition({16 * (playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (z + playerChunk.y - WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddNegX(chunks);
}

void VoxelWorld::stepPlayerPosZ(){
	auto chunks = generateChunksZ(args, playerChunk.x - WORLD_RADIUS, playerChunk.y + WORLD_RADIUS);
	for(int x = 0; x < chunks.size(); x++){
		auto& chunk = chunks[x];
		chunk->setPosition({16 * (x + playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (playerChunk.y + WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddPosZ(chunks);
	playerChunk.y++;
}

void VoxelWorld::stepPlayerNegZ(){
	playerChunk.y--;

	auto chunks = generateChunksZ(args, playerChunk.x - WORLD_RADIUS, playerChunk.y - WORLD_RADIUS);
	for(int x = 0; x < chunks.size(); x++){
		auto& chunk = chunks[x];
		chunk->setPosition({16 * (x + playerChunk.x - WORLD_RADIUS), -CHUNK_Y_SIZE / 2, 16 * (playerChunk.y - WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddNegZ(chunks);
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddPosX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks) {
    for(int i = 0; i < chunks.size(); i++)
        this->chunks[i].emplace_back(chunks[i]);
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddNegX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks) {
    for(int i = 0; i < chunks.size(); i++)
        this->chunks[i].emplace_front(chunks[i]);
}

std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1> VoxelWorld::generateChunksX(const Arguments& args, size_t X, size_t startZ) {
    std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1> out;
    for(int i = 0; i < WORLD_RADIUS * 2 + 1; i++){ // one less, we don't generate the null chunk
        out[i] = std::make_shared<Chunk>();
		generationQueue.emplace(out[i], glm::ivec2{X, startZ + i});
	    // out[i]->generateVoxels(args, X, startZ + i);
    }

    return out;
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddPosZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks) {
    this->chunks.emplace_back(chunks);
}

// NOTE: Expects the last element of the array to be null
void VoxelWorld::AddNegZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks) {
    this->chunks.emplace_front(chunks);
}

std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1> VoxelWorld::generateChunksZ(const Arguments& args, size_t startX, size_t Z) {
    std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1> out;
    for(int i = WORLD_RADIUS * 2; 0 <= i ; i--){ // one less, we don't generate the null chunk
        out[i] = std::make_shared<Chunk>();
		generationQueue.emplace(out[i], glm::ivec2{startX + i, Z});
	    // out[i]->generateVoxels(args, startX + i, Z);
    }

    return out;
}
