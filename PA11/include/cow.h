#ifndef COW_H
#define COW_H

#include "npc.h"

class Cow : public NPC {
public:
    Cow() { typeID = 1; world = nullptr; }
	Cow(std::shared_ptr<VoxelWorld> vw)  { typeID = 1; world = vw; }
    ~Cow();

private:

};

#endif  /* COW_H */
