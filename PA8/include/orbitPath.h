#ifndef ORBITPATH_H
#define ORBITPATH_H

#include "object.h"

// Object representing path of orbit
class OrbitPath: public Object {
public:
	OrbitPath() { glGenBuffers(1, &actualVB); }
	bool InitializeGraphics(const Arguments& args, const std::string& texturePath, glm::vec2 orbitDistanceScaled, glm::vec2 orbitDistanceActual, glm::vec3 orbitalTiltNormal, int depth);
	void Update(unsigned int dt) override;
	void Render(GLint modelMatrix) override;

	// Vertecies and vertex buffer for the actual data
	std::vector<Vertex> actualVertices;
	GLuint actualVB;

	// The resolution of the generated mesh
	int resolution = 512;
	// The inside width of the line
	float width = .1;
	glm::vec2 dimensions;

	// The orbital tilt normal of the parent
	glm::vec3 orbitalTiltNormal;
};

#endif /* end of include guard: ORBITPATH_H */
