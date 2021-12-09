#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include "chunk.h"
#include "circular_buffer.hpp"
#include "monitor.hpp"

#include <queue>
#include <algorithm>
#include <thread>
#include <optional>

#define WORLD_RADIUS 16

// Class which extends a priority queue to allow access to its comparision object
template<class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type>>
struct ModifiablePriorityQueue : public std::priority_queue<T, Container, Compare> {
	using std::priority_queue<T, Container, Compare>::priority_queue;
	Compare& getCompare() { return std::priority_queue<T, Container, Compare>::comp; }
	Container& getContainer() { return std::priority_queue<T, Container, Compare>::c; }
};

struct VoxelWorld {
	struct RaycastResult {
		float closestHitFraction;
		const btCollisionObject* collisionObject;
		int collisionFilterGroup, collisionFilterMask;
		glm::vec3 point, normal;
	};

	VoxelWorld(Arguments& args): args(args) {}
	~VoxelWorld() {
		shouldMeshingThreadRun = false; meshingThread.join();
		shouldCollisionThreadRun = false; collisionThread.join();
	}

	void initialize(glm::ivec2 playerChunk = {0, 0});
    void update(float dt);
    void render(Shader* boundShader);

	glm::ivec2 getPlayerChunkCoordinates(){ return playerChunk; }

	// Functions which update which chunk the player is in and load chunks around them accordingly
	void stepPlayerPosX();
	void stepPlayerNegX();
	void stepPlayerPosZ();
	void stepPlayerNegZ();

	// Functions which access the world
	Chunk::ptr getChunk(glm::ivec2 worldPos);
	Chunk::ptr getChunk(glm::ivec3 worldPos) { return getChunk({worldPos.x, worldPos.z}); }
	std::optional<std::array<std::reference_wrapper<Chunk::Voxel>, CHUNK_HEIGHT>> getColumn(glm::ivec2 worldPos);
	std::optional<std::array<std::reference_wrapper<Chunk::Voxel>, CHUNK_HEIGHT>> getColumn(glm::ivec3 worldPos) { return getColumn({worldPos.x, worldPos.z}); }
	std::optional<std::reference_wrapper<Chunk::Voxel>> getVoxel(glm::ivec3 worldPos);

	// Function which determines the highest Y of the world value given its X and Z coordinate
	std::optional<RaycastResult> raycast(std::pair<glm::vec3, glm::vec3> startEnd, int collisionMask = CollisionGroups::All){ return raycast(startEnd.first, startEnd.second, collisionMask); }
	std::optional<RaycastResult> raycast(glm::vec3 start, glm::vec3 end, int collisionMask = CollisionGroups::All){
		btDiscreteDynamicsWorld::ClosestRayResultCallback callback(toBullet(start), toBullet(end));
		callback.m_collisionFilterMask = collisionMask; // Apply collision mask
		// callback.m_collisionFilterGroup = ~CollisionGroups::All;
		Physics::getSingleton().getWorld().rayTest(toBullet(start), toBullet(end), callback);
		if(!callback.hasHit()) return {};
		return RaycastResult{callback.m_closestHitFraction, callback.m_collisionObject, callback.m_collisionFilterGroup, callback.m_collisionFilterMask, toGLM(callback.m_hitPointWorld), toGLM(callback.m_hitNormalWorld)};
	}
	float getWorldHeight(glm::ivec2 worldPos);
	float getWorldHeight(glm::ivec3 worldPos) { return getWorldHeight({worldPos.x, worldPos.z}); }

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
			glm::vec3 origin = {16 * playerChunk->x, -CHUNK_HEIGHT / 2, 16 * playerChunk->y};
			return glm::length2(a->getPosition() - origin) > glm::length2(b->getPosition() - origin);
		}
		// Overload which allows generation pairs to be passed in
		bool operator() (const std::pair<Chunk::ptr, glm::ivec2>& a, const std::pair<Chunk::ptr, glm::ivec2>& b){ return operator() (a.first, b.first); }
	};

protected:
	Arguments& args;

	// Circular Buffer of Circular Buffers of Chunks
	// Need +1 on the radius to include 0,0
	// Outer = X, Inner = Z
    circular_buffer_array<finalizeable_circular_buffer_array<Chunk::ptr, WORLD_RADIUS * 2 + 1>, WORLD_RADIUS * 2 + 1> chunks; // Outer = X, Inner = Z

	// Vec2 storing the chunk the player is currently in
	glm::ivec2 playerChunk = {0, 0};

	// Queue of chunks that need their data generated
	static ModifiablePriorityQueue<std::pair<Chunk::ptr, glm::ivec2>, std::vector<std::pair<Chunk::ptr, glm::ivec2>>, MeshingSort> generationQueue;

	// Queue of chunks that need to be meshed
	monitor<ModifiablePriorityQueue<Chunk::ptr, std::vector<Chunk::ptr>, MeshingSort>> meshingQueue, collisionQueue;
	// Thread responsible for meshing chunks
	std::thread meshingThread, collisionThread;
	// Bool which checks if the meshing thread should keep running
	bool shouldMeshingThreadRun = true, shouldCollisionThreadRun = true;

	// Queue of chunks which need to be uploaded to the gpu
	std::queue<Chunk::ptr> uploadQueue;
};

#endif // VOXEL_WORLD_H
