#include "planet.h"

void Planet::Update(unsigned int dt) {
	// std::cout << m_reverseOrbit << " - " << m_reverseRotation << " - " << pauseOrbit << " - " << pauseRotation << std::endl;
	if(!pauseOrbit)
		orbitAngle += dt * M_PI/1000 * (m_reverseOrbit ? -1 : 1);
	if(!pauseRotation)
		rotationAngle += dt * M_PI/1000 * (m_reverseRotation ? -1 : 1);

	glm::vec3 translation(cos(orbitAngle) * 5, 0, sin(orbitAngle) * 5);
	setModel( glm::rotate(glm::translate( glm::mat4(1.0f), translation ), (rotationAngle), glm::vec3(0.0, 1.0, 0.0)) );

	Object::Update(dt);
}

void Planet::Keyboard(const SDL_KeyboardEvent& e){
	if(e.type != SDL_KEYDOWN) return;

	if(e.keysym.sym == SDLK_o)
		toggleOrbit();
	else if(e.keysym.sym == SDLK_r)
		toggleRotation();
	else if(e.keysym.sym == SDLK_f)
		reverseRotation();
	else if(e.keysym.sym == SDLK_l)
		reverseOrbit();

	// No reference to base keyboard... we don't pass events to moons
}

void Planet::MouseButton(const SDL_MouseButtonEvent& e){
	if(e.type != SDL_MOUSEBUTTONDOWN) return;

	if(e.button == SDL_BUTTON_LEFT)
		reverseOrbit();
	else if(e.button == SDL_BUTTON_RIGHT)
		reverseRotation();

	// No reference to base keyboard... we don't pass events to moons
}

void Planet::toggleOrbit(){ pauseOrbit = !pauseOrbit; }
void Planet::toggleRotation(){ pauseRotation = !pauseRotation; }
void Planet::reverseOrbit(){ m_reverseOrbit = !m_reverseOrbit; }
void Planet::reverseRotation(){ m_reverseRotation = !m_reverseRotation; }
