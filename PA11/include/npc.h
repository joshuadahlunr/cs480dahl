#ifndef NPC_H
#define NPC_H

#include "object.h"
#include "voxel_world.h"

class NPC : public Object {
public:
    NPC() : world(nullptr) {};
	NPC(std::shared_ptr<VoxelWorld> vw) : world(vw) {};
    
	void update(float dt);
    void clearTargets();

    void setMovementState(bool isMoving);
    void setWaypoint(glm::vec3 newWaypoint) {waypoint = newWaypoint;};

    bool isOnGround();

    bool getIsBeingAbducted() {return isBeingAbducted; };
    void setIsBeingAbducted(bool beingAbducted);
    int getTypeID() { return typeID; };
    void setSpeed(float newSpeed) {speed = newSpeed;};

protected:
    int typeID = 0;
	std::shared_ptr<VoxelWorld> world;

private:
    glm::vec3 waypoint;
    float speed = 50;
    float wanderDistance = 20; // how far a npc could wander per waypoint
    float minTargetDistance = 1;
    float currentWait=0;
    bool canMove = true;
    bool isBeingAbducted = false;
};

#endif  /* NPC_H */
