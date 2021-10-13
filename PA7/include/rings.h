#ifndef RINGS_H
#define RINGS_H

#include "object.h"

// Object representing planetary rings
class Ring: public Object {
public:
	bool Initialize(const Arguments& args, const std::string& texturePath);
	void Render(GLint modelMatrix) override;
	void Update(unsigned int dt) override;

	// Functions which provide scaled versions of the below variables
	float scaledRadius(float r);

	// The resolution of the generated ring mesh
	int resolution = 128;
	// The inside radius of the ring
	float innerRadius = 1;
	// The outside radius of the ring
	float outerRadius = 2;
	// How the ring is tilted relative to its planet
	glm::vec3 tilt = glm::vec3(0, 1, 0);
};

#endif /* end of include guard: RINGS_H */
