#include "npc.h"

NPC::NPC() {}

NPC::~NPC() {}

void NPC::update(float dt) {
	// Add waypoints if none
	if (waypoints.size() < 1) {
		float x = rand() % (int) (wanderDistance * 2 + 1) - (wanderDistance) + getPosition().x;
		float z = rand() % (int) (wanderDistance * 2 + 1) - (wanderDistance) + getPosition().z;
		waypoints.push(glm::vec3(x, 0, z));
		waitTimes.push(rand() % (5));
	}

	if(waypoints.empty()) return;

	// decrement a wait
	if (glm::distance(waypoints.front(), getPosition()) < 1.0f) {
		currentWait -= dt;

		if (currentWait < 0) {
			waypoints.pop();
			currentWait = waitTimes.front();
		}
	} else {
		glm::vec3 direction = glm::normalize(waypoints.front() - getPosition());

		float speed = 5;
		setPosition(getPosition() + (direction * speed * dt));
	}




}

