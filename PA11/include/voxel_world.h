#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include "chunk.h"
#include "circular_buffer.hpp"

#include <queue>
#include <algorithm>

#define WORLD_RADIUS 25

// Class which extends a priority queue to allow access to its comparision object
template<class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type>>
struct ModifiablePriorityQueue : public std::priority_queue<T, Container, Compare> {
	using std::priority_queue<T, Container, Compare>::priority_queue;
	Compare& getCompare() { return std::priority_queue<T, Container, Compare>::comp; }
};

struct VoxelWorld {
	VoxelWorld(Arguments& args): args(args) {}

	void initialize(glm::ivec2 playerChunk = {0, 0});
    void update(float dt);
    void render(Shader* boundShader);

	glm::ivec2 getPlayerChunk(){ return playerChunk; }

	// Functions which update which chunk the player is in and load chunks around them accordingly
	void stepPlayerPosX();
	void stepPlayerNegX();
	void stepPlayerPosZ();
	void stepPlayerNegZ();

protected:
    void AddPosX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks);
    void AddNegX(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks);
    static std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1> generateChunksX(const Arguments& args, size_t X, size_t startZ);

    void AddPosZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks);
    void AddNegZ(const std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1>& chunks);
    static std::array<Chunk::ptr, WORLD_RADIUS * 2 + 1> generateChunksZ(const Arguments& args, size_t startX, size_t Z);

	// Structure which sorts chunks based on their distance to the player's chunk
	struct MeshingSort {
		glm::ivec2* playerChunk;

		bool operator() (const Chunk::ptr& a, const Chunk::ptr& b){
			glm::vec3 origin = {16 * playerChunk->x, -CHUNK_Y_SIZE / 2, 16 * playerChunk->y};
			return glm::length2(a->getPosition() - origin) > glm::length2(b->getPosition() - origin);
		}
		// Overload which allows generation pairs to be passed in
		bool operator() (const std::pair<Chunk::ptr, glm::ivec2>& a, const std::pair<Chunk::ptr, glm::ivec2>& b){ return operator() (a.first, b.first); }
	};

protected:
	Arguments& args;

	// Circular Buffer of Circular Buffers of Chunks
	// Need +1 on the radius to include 0,0 and another +1 for the empty slot the buffer requires
    circular_buffer_array<circular_buffer_array<Chunk::ptr, WORLD_RADIUS * 2 + 1>, WORLD_RADIUS * 2 + 1> chunks; // Outer = X, Inner = Z

	// Vec2 storing the chunk the player is currently in
	glm::ivec2 playerChunk = {0, 0};

	// Queue of chunks that need their data generated
	static ModifiablePriorityQueue<std::pair<Chunk::ptr, glm::ivec2>, std::vector<std::pair<Chunk::ptr, glm::ivec2>>, MeshingSort> generationQueue;

	// Queue of chunks that need to be meshed
	ModifiablePriorityQueue<Chunk::ptr, std::vector<Chunk::ptr>, MeshingSort> meshingQueue;
};

#endif // VOXEL_WORLD_H
