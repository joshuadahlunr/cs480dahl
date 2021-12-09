#include "voxel_world.h"

// Macros for accessing glm::vec2s which represent points in x, z space
#define X(variable) (variable).x
#define Z(variable) (variable).y

// Memory backing for the generation queue
ModifiablePriorityQueue<std::pair<Chunk::ptr, glm::ivec2>, std::vector<std::pair<Chunk::ptr, glm::ivec2>>, VoxelWorld::MeshingSort> VoxelWorld::generationQueue;

// Outer = X, Inner = Z

void VoxelWorld::initialize(glm::ivec2 playerChunk /*= {0, 0}*/){
	this->playerChunk = playerChunk;
	generationQueue.getCompare().playerChunk = &this->playerChunk;
	meshingQueue->getCompare().playerChunk = &this->playerChunk;
	collisionQueue->getCompare().playerChunk = &this->playerChunk;

	for(int z = Z(playerChunk) - WORLD_RADIUS; z <= Z(playerChunk) + WORLD_RADIUS; z++){
		auto chunks = generateChunksZ(args, X(playerChunk) - WORLD_RADIUS, z);
		for(int x = 0; x < chunks.size(); x++){
			auto& chunk = chunks[x];
			chunk->setPosition({(CHUNK_WIDTH - 1) * (x + X(playerChunk) - WORLD_RADIUS), -CHUNK_HEIGHT / 2, (CHUNK_WIDTH - 1) * (z + Z(playerChunk))});
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
				// Prep the mesh for collisions
				collisionQueue->push(nextMesh);

			// If there aren't chunks to generate... sleep for 5 milliseconds
			} else std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	});

	// Start a new collision thread which just generates colliders for chunks while there are chunks without colliders
	collisionThread = std::thread([this](){
		while(shouldCollisionThreadRun){
			// If there are chunks which need colliders generated for them... generate a collider for those chunks
			if(!collisionQueue.unsafe().empty()){
				auto nextMesh = collisionQueue.read_lock()->top();
				collisionQueue->pop();

				// Sleep until the mesh has been uploaded to the gpu
				while(nextMesh->state != Chunk::GenerateState::Finalized)
					std::this_thread::sleep_for(std::chrono::milliseconds(5));

				// Wait a moment
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				// Generate the chunk's collision mesh
				nextMesh->initializePhysics(args, Physics::getSingleton(), true, 1'000'000);
				nextMesh->createMeshCollider(args, Physics::getSingleton(), CONCAVE_MESH);
				nextMesh->makeStatic();

			// If there aren't colliders to generate... sleep for 5 milliseconds
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
	auto chunks = generateChunksX(args, X(playerChunk) + WORLD_RADIUS, Z(playerChunk) - WORLD_RADIUS);
	for(int z = 0; z < chunks.size(); z++){
		auto& chunk = chunks[z];
		chunk->setPosition({(CHUNK_WIDTH - 1) * (X(playerChunk) + WORLD_RADIUS), -CHUNK_HEIGHT / 2, (CHUNK_WIDTH - 1) * (z + Z(playerChunk) - WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddPosX(chunks);
	X(playerChunk)++;
}

void VoxelWorld::stepPlayerNegX(){
	X(playerChunk)--;

	auto chunks = generateChunksX(args, X(playerChunk) - WORLD_RADIUS, Z(playerChunk) - WORLD_RADIUS);
	for(int z = 0; z < chunks.size(); z++){
		auto& chunk = chunks[z];
		chunk->setPosition({(CHUNK_WIDTH - 1) * (X(playerChunk) - WORLD_RADIUS), -CHUNK_HEIGHT / 2, (CHUNK_WIDTH - 1) * (z + Z(playerChunk) - WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddNegX(chunks);
}

void VoxelWorld::stepPlayerPosZ(){
	auto chunks = generateChunksZ(args, X(playerChunk) - WORLD_RADIUS, Z(playerChunk) + WORLD_RADIUS);
	for(int x = 0; x < chunks.size(); x++){
		auto& chunk = chunks[x];
		chunk->setPosition({(CHUNK_WIDTH - 1) * (x + X(playerChunk) - WORLD_RADIUS), -CHUNK_HEIGHT / 2, (CHUNK_WIDTH - 1) * (Z(playerChunk) + WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddPosZ(chunks);
	Z(playerChunk)++;
}

void VoxelWorld::stepPlayerNegZ(){
	Z(playerChunk)--;

	auto chunks = generateChunksZ(args, X(playerChunk) - WORLD_RADIUS, Z(playerChunk) - WORLD_RADIUS);
	for(int x = 0; x < chunks.size(); x++){
		auto& chunk = chunks[x];
		chunk->setPosition({(CHUNK_WIDTH - 1) * (x + X(playerChunk) - WORLD_RADIUS), -CHUNK_HEIGHT / 2, (CHUNK_WIDTH - 1) * (Z(playerChunk) - WORLD_RADIUS)});
	}

	// Resort generation queue so generation happens around the player
	std::sort(generationQueue.getContainer().begin(), generationQueue.getContainer().end(), generationQueue.getCompare());

	AddNegZ(chunks);
}


// Function which extracts the chunk at the given world position
Chunk::ptr VoxelWorld::getChunk(glm::ivec2 worldPos){
	glm::ivec2 chunkPos = worldPos / (CHUNK_WIDTH - 1);	// Convert from world space to chunk space
	if(X(worldPos) < 0) X(chunkPos)--;
	if(Z(worldPos) < 0) Z(chunkPos)--;
	chunkPos -= playerChunk;				// Make chunk position relative to the player's position

	// If the requested chunk is currently outside of the loaded radius, return nullptr
	if(glm::length2(glm::vec2(chunkPos)) > WORLD_RADIUS * WORLD_RADIUS)
		return nullptr;

	chunkPos = glm::ivec2(WORLD_RADIUS) - chunkPos;	// Normalize the coordinates into array indices

	return chunks[X(chunkPos)][Z(chunkPos)];
}

template<typename T, size_t size>
std::array<T, size> fillInitalize(T value){
	auto arrayGenerator = []<size_t... I>(T& value, std::integer_sequence<size_t, I...> s){
		auto fillGenerator = [](int index, T& value) { return value; };
		return std::array<T, sizeof...(I)> { fillGenerator(I, value)... };
	};
	
	return arrayGenerator(value, std::make_index_sequence<size>{});
}

// Function which extracts the column of voxels at the given world position
std::optional<std::array<std::reference_wrapper<Chunk::Voxel>, CHUNK_HEIGHT>> VoxelWorld::getColumn(glm::ivec2 worldPos){
	auto chunk = getChunk(worldPos);
	// If the chunk is invalid, return an invalid optional
	if(!chunk) return {};

	// Normalize the chunk positions [0, 16)
	glm::ivec2 innerChunkPos = worldPos % (CHUNK_WIDTH - 1);
	if(X(innerChunkPos) < 0) X(innerChunkPos) += CHUNK_WIDTH;
	if(Z(innerChunkPos) < 0) Z(innerChunkPos) += CHUNK_WIDTH;

	// Initalize the array with a given default value
	auto column = fillInitalize<std::reference_wrapper<Chunk::Voxel>, CHUNK_HEIGHT>(std::ref(chunk->voxels[0][0][0]));
	// Fill the array with references to the individual voxels
	for(int y = 0; y < CHUNK_HEIGHT; y++)
		column[y] = std::ref(chunk->voxels[X(innerChunkPos)][y][Z(innerChunkPos)]);
 
	return column;
}

// Function which extracts the voxel at a given world position
std::optional<std::reference_wrapper<Chunk::Voxel>> VoxelWorld::getVoxel(glm::ivec3 worldPos){
	worldPos.y += CHUNK_HEIGHT / 2; // The whole world is shifted by half (y = 0 in chunks = y = -128 in world)
	// If the y value is outside the valid range, return invalid
	if(worldPos.y < 0 || worldPos.y > CHUNK_HEIGHT) return {};
	auto chunk = getChunk(worldPos);
	// If the chunk is not loaded, return invalid
	if(!chunk) return {};

	// Normalize the chunk positions [0, 16)
	glm::ivec3 innerChunkPos = { worldPos.x % (CHUNK_WIDTH - 1), worldPos.y, worldPos.z % (CHUNK_WIDTH - 1) };
	if(innerChunkPos.x < 0) innerChunkPos.x += CHUNK_WIDTH;
	if(innerChunkPos.z < 0) innerChunkPos.z += CHUNK_WIDTH;

	return chunk->voxels[innerChunkPos.x][innerChunkPos.y][innerChunkPos.z];
}


// Function which determines the highest Y of the world value given its X and Z coordinate
float VoxelWorld::getWorldHeight(glm::ivec2 worldPos){
	auto columnOpt = getColumn(worldPos);
	if(!columnOpt) return 0;
	auto column = std::move(columnOpt.value());

	size_t top = 255;
	Chunk::Voxel* voxel;
	for( ; top > 0; top--)
		if(voxel = &column[top].get(); voxel->type != Chunk::Voxel::Type::Air)
			break;
	
	float level = top - voxel->isoLevel;	// The loop will get us the bottom of the voxel, then we need to subtract its ISO level to get the surface level
	level -= CHUNK_HEIGHT / 2;				// The whole world is shifted by half (y = 0 in chunks = y = -128 in world)
	return level;
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
