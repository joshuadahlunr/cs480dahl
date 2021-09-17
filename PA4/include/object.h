#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <SDL2/SDL.h>
#include "graphics_headers.h"
#include "arguments.h"

class Object {
public:
	Object();
	~Object();
	virtual void Initialize(const Arguments& args);
	virtual void Update(unsigned int dt);
	virtual void Render(GLint modelMatrix);

	virtual void Keyboard(const SDL_KeyboardEvent& e);
	virtual void MouseButton(const SDL_MouseButtonEvent& e);

	Object* setParent(Object* p);
	Object* getParent() const { return parent; }
	Object* addChild(Object* child);
	const std::vector<Object*>& getChildren() const { return children; }

	glm::mat4 GetModel() { return model; }
	void setModel(glm::mat4 _model) { model = _model; }
	void setModelRelativeToParent(glm::mat4 _model);

protected:
	void LoadOBJFile(const std::string& path, glm::mat4 onImportTransformation = glm::mat4(1));

protected:
	glm::mat4 model;
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
	GLuint VB;
	GLuint IB;

	Object* parent = nullptr;
	std::vector<Object*> children;
};

#endif /* OBJECT_H */
