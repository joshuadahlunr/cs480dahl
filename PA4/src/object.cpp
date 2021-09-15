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

void Object::Update(unsigned int dt){
	for(Object* child: children)
		child->Update(dt);
}

void Object::Render(GLint modelMatrix) {
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, glm::value_ptr(GetModel()));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	for(Object* child: children)
		child->Render(modelMatrix);
}

Object* Object::setParent(Object* p){
	if(parent == p) return p;

	if(parent != nullptr)
		for(int i = 0; i < parent->children.size(); i++)
			if(parent->children[i] == this){ // TODO: Are pointer comparisons sufficient here?
				parent->children.erase(parent->children.begin() + i);
				break;
			}

	parent = p;
	p->addChild(this);
	return p;
}

Object* Object::addChild(Object* child){
	// If the object is already a child... don't bother adding
	for(Object* c: children)
		if(c == child)
			return child;

	children.push_back(child);
	child->setParent(this);
	return child;
}

void Object::Keyboard(const SDL_KeyboardEvent& e){
	for(Object* child: children)
		child->Keyboard(e);
}

void Object::MouseButton(const SDL_MouseButtonEvent& e){
	for(Object* child: children)
		child->MouseButton(e);
}

void Object::setModelRelativeToParent(glm::mat4 _model){
	model = (parent ? parent->model : glm::mat4(1)) * _model;
}
