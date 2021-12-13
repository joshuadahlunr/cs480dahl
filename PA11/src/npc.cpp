#include "npc.h"

void NPC::update(float dt) {
	// Call the base update
	Object::update(dt);

	// Set the y position to actual position 

	// decrement a wait
	currentWait -= dt;

	//float height = Application::getWorld()->getWorldHeight();

	if (canMove && isOnGround()) {
		if (currentWait < 0) {
			float x = ( rand() % (int) (wanderDistance)) - (wanderDistance * 0.5) + getPosition().x;
			float z = ( rand() % (int) (wanderDistance)) - (wanderDistance * 0.5) + getPosition().z;
			// Make new waypoint 
			glm::ivec3 pos = (glm::ivec3) getPosition();
			if (isnan(world->getWorldHeight(pos)))
				waypoint = getPosition();
			else {
				waypoint = glm::vec3(x, world->getWorldHeight(pos), z);
				// Check if waypoint is too far of a step up or down
				if (std::abs(waypoint.y - getPosition().y) > 5) {
					waypoint = getPosition();
				}
			}
			currentWait = rand() % (10);
		} else {
			glm::vec3 direction = glm::normalize(waypoint - getPosition());
			float angle = std::atan2(getLinearVelocity().z, getLinearVelocity().x);

			if (glm::length(getLinearVelocity()) > 0.5f) 
				setRotation(glm::quat(glm::vec3(0,-angle, 0)));
			//rotate(glm::degrees(angle), glm::vec3(0,1,0));

			speed = 5;
			// setPosition(getPosition() + (direction * speed * dt));
			applyForce(direction * speed * 100.0f);
		}
	}
}

bool NPC::isOnGround() {
	//Application::getWorld()->getWorldHeight();
	glm::ivec3 pos = (glm::ivec3) getPosition();
	if (isnan(world->getWorldHeight(pos))) return false;
	
	float distance = std::abs(pos.y - world->getWorldHeight(pos));
	//std::cout << distance << std::endl;
	if (distance > 1.0f) {
		return false;
	}
	
	return true;
}

void NPC::setMovementState(bool isMoving) {
	canMove = isMoving;
}

void NPC::clearTargets() {
	waypoint = glm::vec3();
	currentWait = -1;
}

void NPC::setIsBeingAbducted(bool beingAbducted) {
	if (beingAbducted) {
		getRigidBody().setGravity({0, 0, 0});
		setMovementState(false);
		setAngularVelocity(glm::vec3((rand() % 2) - 1, (rand() % 2) - 1, (rand() % 2) - 1));
	} else {
		getRigidBody().setGravity({0, -9.81, 0});
		setMovementState(true);
		//setAngularVelocity(glm::vec3());
	}
	isBeingAbducted = beingAbducted;
}

