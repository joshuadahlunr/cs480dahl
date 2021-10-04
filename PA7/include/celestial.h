#ifndef CELESTIAL_H
#define CELESTIAL_H

#include "object.h"

const float milliToSec = 1.0/1000.0;

class Celestial: public Object {
public:
	bool Initialize(const Arguments& args, const std::string& texturePath);
	void Update(unsigned int dt) override;

	float celestialRadius = 1;
	float orbitDistance = 5;
	float orbitSpeed = 36; 
	float orbitInitialOffset = 0;
	glm::vec3 orbitalTiltNormal = glm::vec3(0, 1, 0);
	float rotationSpeed = 1;
	glm::vec3 axialTiltNormal = glm::vec3(0, 1, 0);

	float orbitAngle = 90 + orbitInitialOffset, rotationAngle = 0;
};

#endif
