#ifndef ORBITPATH_H
#define ORBITPATH_H

#include "object.h"

// Object representing path of orbit
class OrbitPath: public Object {
public:
	bool Initialize(const Arguments& args, const std::string& texturePath, glm::vec2 orbitDistance, glm::vec3 orbitalTiltNormal);
	void Render(GLint modelMatrix) override;

	// The resolution of the generated mesh
	int resolution = 512;
	// The inside width of the line
	float width = .05;
	glm::vec2 dimensions;
};

#endif /* end of include guard: ORBITPATH_H */
