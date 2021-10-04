#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include <SDL2/SDL.h>
#include "graphics_headers.h"
#include "arguments.h"

#include <assimp/Importer.hpp> //includes the importer, which is used to read our obj file
#include <assimp/scene.h> //includes the aiScene object
#include <assimp/postprocess.h> //includes the postprocessing variables for the importer
#include <assimp/color4.h> //includes the aiColor4 object, which is used to handle the colors from the mesh objects

class Object {
public:
	Object();
	~Object();
	virtual bool Initialize(const Arguments& args);
	virtual void Update(unsigned int dt);
	virtual void Render(GLint modelMatrix);

	virtual void Keyboard(const SDL_KeyboardEvent& e);
	virtual void MouseButton(const SDL_MouseButtonEvent& e);

	Object* setParent(Object* p);
	Object* getParent() const { return parent; }
	Object* addChild(Object* child);
	const std::vector<Object*>& getChildren() const { return children; }

	glm::mat4 GetModel() { return model; }
	void setModel(glm::mat4 _model) { childModel = model = _model; }
	void setChildModel(glm::mat4 _model) { childModel = _model; }
	void setModelRelativeToParent(glm::mat4 _model);
	void setChildModelRelativeToParent(glm::mat4 _model);

protected:
	bool LoadModelFile(const Arguments& args, const std::string& path, glm::mat4 onImportTransformation = glm::mat4(1));
	bool LoadTextureFile(const Arguments& args, std::string path, bool makeRelative = true);

protected:
	glm::mat4 model;
	glm::mat4 childModel; // Model that is used as the base of to this object's children's model matricies
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
	GLuint VB;
	GLuint IB;
	GLuint tex = -1;

	Object* parent;
	std::vector<Object*> children;
};

#endif /* OBJECT_H */
