#ifndef CHUNK_H
#define CHUNK_H

#include "object.h"
#define NOISE_SEED 12345

#define CHUNK_WIDTH 17
#define CHUNK_HEIGHT 256

#define TREE_MAX_ANGLE 0.0872665
#define TREE_SPARCITY 100

struct Chunk : public Object {
    using ptr = std::shared_ptr<Chunk>;

    // Struct representing a voxel
    struct Voxel {
        enum Type : uint32_t {
            Air = 0,
            Grass = 1,
            Stone = 2
        };

        Type type;
        float isoLevel;
    };

    // Enum tracking the current generation state
    enum GenerateState {
        NotStarted,
        Freed,
        Generated,
        Meshed,
        Finalized
    } state = NotStarted;

    // Only render the chunk if it has finished being generated
    void render(Shader* boundShader) override { if(state == Finalized) Object::render(boundShader); }

    // TODO: Chunk width
    // TODO: See if riged perlin noise can generate caves?

    void generateVoxels(const Arguments& args, int x, int z);
    void rebuildMesh(const Arguments& args);
    void generateTrees(const Arguments& args);

    Voxel voxels[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH]; // X, Y, Z
};

#endif // CHUNK_H
