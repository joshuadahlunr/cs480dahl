#ifndef CELESTIAL_H
#define CELESTIAL_H

#include "object.h"

const float milliToSec = 1.0/1000.0;

// Object extension representing a planatry body
class Celestial: public Object {
public:
	// Must be initialized with the desired texture
	bool Initialize(const Arguments& args, const std::string& texturePath);
	void Update(unsigned int dt) override;

	// Functions which provide scaled versions of the below variables
	float scaledRadius();
	glm::vec2 scaledOrbitDistance();

	// Radius of the object
	float celestialRadius = 1;
	// Distance at which object orbits its parent (both x and y)
	glm::vec2 orbitDistance = glm::vec2(0);
	// Speed at which the object orbits its parent
	double orbitSpeed = 0;
	// Initial offset of the object's orbit along its parent
	float orbitInitialOffset = 0;
	// Tilt of the orbit
	float eclipticInclination = 0;
	glm::vec3 orbitalTiltNormal = glm::vec3(0, 1, 0);

	// Speed of Rotation
	float rotationSpeed = 0;
	// Axial tilt of the planet
	glm::vec3 axialTiltNormal = glm::vec3(0, 1, 0);

protected:
	// Variables tracking the current orbit and rotation angle of the planet over time
	float orbitAngle = 90 + orbitInitialOffset, rotationAngle = 0;

};

#endif
