#ifndef PLANET_H
#define PLANET_H

#include "object.h"

class Planet: public Object {
public:
	void Update(unsigned int dt) override;

	void Keyboard(const SDL_KeyboardEvent& e) override;
	void MouseButton(const SDL_MouseButtonEvent& e) override;

	void toggleOrbit();
	void reverseOrbit();
	void toggleRotation();
	void reverseRotation();

	bool orbitIsPaused() const { return pauseOrbit; }
	bool rotationIsPaused() const { return pauseRotation; }
	bool orbitIsReversed() const { return m_reverseOrbit; }
	bool rotationIsReversed() const { return m_reverseRotation; }

	void setOrbitSpeed(float speed) { orbitSpeed = speed; }
	float getOrbitSpeed() { return orbitSpeed; }
	void setRotationSpeed(float speed) { rotationSpeed = speed; }
	float getRotationSpeed() { return rotationSpeed; }

private:
	float orbitAngle = 0, rotationAngle = 0;
	bool pauseOrbit = false, pauseRotation = false;
	bool m_reverseOrbit = false, m_reverseRotation = false;

	float orbitSpeed = 1, rotationSpeed = 1;
};

#endif
