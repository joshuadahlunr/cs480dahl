#ifndef CHUNK_H
#define CHUNK_H

#include "object.h"
#define NOISE_SEED 12345

#define CHUNK_X_SIZE 17
#define CHUNK_Y_SIZE 257
#define CHUNK_Z_SIZE 17

struct Chunk : public Object {

    using ptr = std::shared_ptr<Chunk>;

    struct Voxel {
        enum Type : uint8_t {
            Air = 0,
            Grass = 1
        };

        Type id;
        float isoLevel;
    };

    // TODO: Chunk width
    // TODO: See if riged perlin noise can generate caves?

    void generateVoxels(int x, int z);

    void rebuildMesh(const Arguments& args);

    Voxel voxels[CHUNK_X_SIZE][CHUNK_Y_SIZE][CHUNK_Z_SIZE]; // X, Y, Z
};

#endif // CHUNK_H
