#ifndef ALIEN_H
#define ALIEN_H

#include "npc.h"

class Alien : public NPC {
public:
	//using ptr = std::shared_ptr<Alien>;
	Alien() { typeID = 2; world = nullptr; }
	Alien(std::shared_ptr<VoxelWorld> vw)  { typeID = 2; world = vw; }
	~Alien();

private:

};

#endif  /* COW_H */
