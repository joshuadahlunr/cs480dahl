#ifndef NPC_H
#define NPC_H

#include "object.h"

class NPC : public Object {
public:
	NPC();

	virtual void update(float dt);
    void clearTargets();

    void setMovementState(bool isMoving);

private:
    glm::vec3 waypoint;

    float wanderDistance = 20; // how far a npc could wander per waypoint
    float minTargetDistance = 1;
    float currentWait=0;
    bool canMove = true;
};

#endif  /* NPC_H */
