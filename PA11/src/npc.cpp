#include "npc.h"

NPC::NPC() {}

void NPC::update(float dt) {
	// Call the base update
	Object::update(dt);

	// Set the y position to actual position 
	waypoint.y = getPosition().y;

	// decrement a wait
	currentWait -= dt;

	if (currentWait < 0) {
		float x = ( rand() % (int) (wanderDistance)) - (wanderDistance * 0.5) + getPosition().x;
		float z = ( rand() % (int) (wanderDistance)) - (wanderDistance * 0.5) + getPosition().z;
		waypoint = glm::vec3(x, getPosition().y, z);
		currentWait = rand() % (10);;
	} else {
		glm::vec3 direction = glm::normalize(waypoint - getPosition());
		float angle = std::atan2(getLinearVelocity().x, getLinearVelocity().z);

		//if (glm::length(getLinearVelocity()) > 0.5f) 
			setRotation(glm::quat(glm::vec3(0, angle, 0)));
		//rotate(glm::degrees(angle), glm::vec3(0,1,0));

		float speed = 5;
		// setPosition(getPosition() + (direction * speed * dt));
		applyForce(direction * speed * 100.0f);
	}
}

void NPC::clearTargets() {
	waypoint = glm::vec3();
	currentWait = -1;
}

