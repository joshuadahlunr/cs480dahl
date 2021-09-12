#include "moon.h"

void Moon::Update(unsigned int dt) {
	// minus so it orbits in the opposite direction of its parent
	angle -= dt * M_PI/1000 * .5;

	glm::vec3 translation(cos(angle) * 5, 0, sin(angle) * 5);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
	model *= glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0, 1.0, 0.0));
	model *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
	setModelRelativeToParent( model );

	Object::Update(dt);
}
