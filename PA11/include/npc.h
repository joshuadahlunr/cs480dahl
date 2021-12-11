#ifndef NPC_H
#define NPC_H

#include "chunk.h"
#include "object.h"
#include <queue>


// Object representing a shader program
class NPC : public Object {
public:
	NPC();

	void update(float dt);
    void clearTargets();

private:
    std::queue<glm::vec3> waypoints;
    std::queue<float> waitTimes;

    float wanderDistance = 20; // how far a npc could wander per waypoint
    float minTargetDistance = 1;
    float currentWait=0;
};

#endif  /* NPC_H */
