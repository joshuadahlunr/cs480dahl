#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <SDL2/SDL.h>
#include "graphics_headers.h"
#include "arguments.h"

// Base class for objects in the scene tree, provides basic mouse, keyboard, and
// tick event propagation, along with managing the model, texture, and position in the scene tree of the object
class Object {
public:
	Object();
	~Object();
	virtual bool InitializeGraphics(const Arguments& args);
	virtual void Update(unsigned int dt);
	virtual void Render(GLint modelMatrix);

	// Mouse and Keyboard event propagation
	virtual void Keyboard(const SDL_KeyboardEvent& e);
	virtual void MouseButton(const SDL_MouseButtonEvent& e);

	// Scene tree management
	Object* setParent(Object* p);
	Object* getParent() const { return parent; }
	Object* addChild(Object* child);
	const std::vector<Object*>& getChildren() const { return children; }

	// Sets model matrix
	glm::mat4 GetModel() { return model; }
	glm::mat4 GetChildBaseModel() { return childModel;  }
	glm::vec3 GetPosition() { return glm::vec3(model[3][0], model[3][1], model[3][2]); }
	void setModel(glm::mat4 _model) { childModel = model = _model; }
	void setChildModel(glm::mat4 _model) { childModel = _model; }
	void setModelRelativeToParent(glm::mat4 _model);
	void setChildModelRelativeToParent(glm::mat4 _model);

	// The depth in the scene tree of this object
	uint sceneDepth = 0;

protected:
	// Uploads the model data to the GPU
	void FinalizeModel();
	// Model/Texture loading
	bool LoadModelFile(const Arguments& args, const std::string& path, glm::mat4 onImportTransformation = glm::mat4(1));
	bool LoadTextureFile(const Arguments& args, std::string path, bool makeRelative = true);

protected:
	glm::mat4 model;
	glm::mat4 childModel; // Model matrix that is used as the base of to this object's children's model matricies
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
	GLuint VB;
	GLuint IB;
	GLuint tex = -1;

	Object* parent;
	std::vector<Object*> children;
};

// Objects which do nothing when initialized and link their model matrix to their parent each frame
// (used to hold data for models with multiple materials/submeshes)
class Submesh: public Object {
public:
	using Object::Object;

	// Submesh initialization doesn't do anything
	bool InitializeGraphics(const Arguments& args) override { return true; }
	// A submesh syncs it model matrix to its parent every frame
	void Update(unsigned int dt) override { setModelRelativeToParent(glm::mat4(1)); }
};

#endif /* OBJECT_H */
