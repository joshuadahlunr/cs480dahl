#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <SDL2/SDL.h>
#include "graphics_headers.h"

class Object {
public:
	Object();
	~Object();
	void Update(unsigned int dt);
	void Render();

	void Keyboard(const SDL_KeyboardEvent& e);
	void MouseButton(const SDL_MouseButtonEvent& e);

	void toggleOrbit();
	void reverseOrbit();
	void toggleRotation();
	void reverseRotation();

	bool orbitIsPaused() { return pauseOrbit; }
	bool rotationIsPaused() { return pauseRotation; }
	bool orbitIsReversed() { return m_reverseOrbit; }
	bool rotationIsReversed() { return m_reverseRotation; }

	glm::mat4 GetModel();

private:
	glm::mat4 model;
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
	GLuint VB;
	GLuint IB;

	float orbitAngle, rotationAngle;
	bool pauseOrbit, pauseRotation;
	bool m_reverseOrbit, m_reverseRotation;
};

#endif /* OBJECT_H */
