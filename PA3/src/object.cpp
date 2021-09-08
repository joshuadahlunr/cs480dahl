#include "object.h"

Object::Object() {
	/*
		# Blender File for a Cube
		o Cube
		v 1.000000 -1.000000 -1.000000
		v 1.000000 -1.000000 1.000000
		v -1.000000 -1.000000 1.000000
		v -1.000000 -1.000000 -1.000000
		v 1.000000 1.000000 -0.999999
		v 0.999999 1.000000 1.000001
		v -1.000000 1.000000 1.000000
		v -1.000000 1.000000 -1.000000
		s off
		f 2 3 4
		f 8 7 6
		f 1 5 6
		f 2 6 7
		f 7 8 4
		f 1 4 8
		f 1 2 4
		f 5 8 6
		f 2 1 6
		f 3 2 7
		f 3 7 4
		f 5 1 8
	*/

	Vertices = {
		{{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}},
		{{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
		{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
		{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}}
	};

	Indices = {
		2, 3, 4,
		8, 7, 6,
		1, 5, 6,
		2, 6, 7,
		7, 8, 4,
		1, 4, 8,
		1, 2, 4,
		5, 8, 6,
		2, 1, 6,
		3, 2, 7,
		3, 7, 4,
		5, 1, 8
	};

	// The index works at a 0th index
	for(unsigned int i = 0; i < Indices.size(); i++)
		Indices[i] = Indices[i] - 1;

	rotationAngle = orbitAngle = 0.0f;
	m_reverseOrbit = m_reverseRotation = pauseOrbit = pauseRotation = false;

	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
}

Object::~Object() {
	Vertices.clear();
	Indices.clear();
}

void Object::Update(unsigned int dt) {
	// std::cout << m_reverseOrbit << " - " << m_reverseRotation << " - " << pauseOrbit << " - " << pauseRotation << std::endl;
	if(!pauseOrbit)
		orbitAngle += dt * M_PI/1000 * (m_reverseOrbit ? -1 : 1);
	if(!pauseRotation)
		rotationAngle += dt * M_PI/1000 * (m_reverseRotation ? -1 : 1);

	glm::vec3 translation(cos(orbitAngle) * 5, 0, sin(orbitAngle) * 5);
	model = glm::rotate(glm::translate( glm::mat4(1.0f), translation ), (rotationAngle), glm::vec3(0.0, 1.0, 0.0));
}

glm::mat4 Object::GetModel() { return model; }

void Object::Render() {
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void Object::Keyboard(const SDL_KeyboardEvent& e){
	if(e.type != SDL_KEYDOWN) return;

	if(e.keysym.sym == SDLK_o)
		toggleOrbit();
	else if(e.keysym.sym == SDLK_r)
		toggleRotation();
	else if(e.keysym.sym == SDLK_f)
		reverseRotation();
	else if(e.keysym.sym == SDLK_l)
		reverseOrbit();
}

void Object::MouseButton(const SDL_MouseButtonEvent& e){
	if(e.type != SDL_MOUSEBUTTONDOWN) return;

	if(e.button == SDL_BUTTON_LEFT)
		reverseOrbit();
	else if(e.button == SDL_BUTTON_RIGHT)
		reverseRotation();
}

void Object::toggleOrbit(){ pauseOrbit = !pauseOrbit; }
void Object::toggleRotation(){ pauseRotation = !pauseRotation; }
void Object::reverseOrbit(){ m_reverseOrbit = !m_reverseOrbit; }
void Object::reverseRotation(){ m_reverseRotation = !m_reverseRotation; }
