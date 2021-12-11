#include "npc.h"

NPC::NPC() {}

void NPC::update(float dt) {
	// Call the base update
	Object::update(dt);

	// Add waypoints if none
	if (waypoints.size() < 1) {
		float x = ( rand() % (int) (wanderDistance)) - (wanderDistance * 0.5) + getPosition().x;
		float z = ( rand() % (int) (wanderDistance)) - (wanderDistance * 0.5) + getPosition().z;
		waypoints.push(glm::vec3(x, getPosition().y, z));
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
		float angle = getRotation().y;

		setRotation(glm::quat(glm::vec3(glm::sin(angle),0,glm::cos(angle)), direction), true);


		float speed = 5;
		// setPosition(getPosition() + (direction * speed * dt));
		setPosition(getPosition() + direction * speed * dt);
	}
}

void NPC::clearTargets() {
	waypoints = std::queue<glm::vec3>();
	waitTimes = std::queue<float>();
}

