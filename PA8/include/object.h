#ifndef OBJECT_LOCAL_H
#define OBJECT_LOCAL_H

#include <vector>
#include <SDL2/SDL.h>
#include "physics.h"
#include "graphics_headers.h"
#include "arguments.h"

// Base class for objects in the scene tree, provides basic mouse, keyboard, and
// tick event propagation, along with managing the model, texture, and position in the scene tree of the object
class Object {
public:
	Object();
	~Object();
	virtual bool InitializeGraphics(const Arguments& args, std::string filepath = "", std::string texturePath = "invalid.png");
	virtual bool InitializePhysics(const Arguments& args, Physics& physics, bool _static);
	virtual void Update(unsigned int dt);
	virtual void Render(GLint modelMatrix);

	// Mouse and Keyboard event propagation
	virtual void Keyboard(const SDL_KeyboardEvent& e);
	virtual void MouseButton(const SDL_MouseButtonEvent& e);

	// Physics functions
	void setPhysicsTransform(rp3d::Transform t) { if(rigidBody) rigidBody->setTransform(t); }
	rp3d::Transform getPhysicsTransform(); // Gets the transform stored in the physics simulation (synced with graphics in update)
	rp3d::Transform getGraphicsTransform(); // Gets the transformed displayed in the graphics visualization (synced with physics in update)
	rp3d::RigidBody& getRigidBody() { return *rigidBody; }
	rp3d::Collider& getCollider() { return *collider; }

	// Physics Collider adding functions
	void addCapsuleCollider(float radius, float height, rp3d::Transform transform = rp3d::Transform());
	void addBoxCollider(glm::vec3 halfExtents, rp3d::Transform transform = rp3d::Transform());
	void addSphereCollider(float radius, rp3d::Transform transform = rp3d::Transform());
	void addMeshCollider(bool makeConvex = true, rp3d::Transform transform = rp3d::Transform());

	// Scene tree management
	Object* setParent(Object* p);
	Object* getParent() const { return parent; }
	Object* addChild(Object* child);
	const std::vector<Object*>& getChildren() const { return children; }

	// Sets model matrix
	glm::mat4 GetModel() { return model; }
	glm::mat4 GetChildBaseModel() { return childModel;  }
	glm::vec3 getPosition() { return glm::vec3(model[3][0], model[3][1], model[3][2]); }
	void setModel(glm::mat4 _model) { childModel = model = _model; }
	void setChildModel(glm::mat4 _model) { childModel = _model; }
	void setModelRelativeToParent(glm::mat4 _model);
	void setChildModelRelativeToParent(glm::mat4 _model);
	void setPosition(glm::vec3 _pos) { model[3][0] = _pos[0]; model[3][1] = _pos[1]; model[3][2] = _pos[2]; }

	// The depth in the scene tree of this object
	uint sceneDepth = 0;

protected:
	// Uploads the model data to the GPU
	void FinalizeModel();
	// Model/Texture loading
	bool LoadModelFile(const Arguments& args, const std::string& path, glm::mat4 onImportTransformation = glm::mat4(1));
	bool LoadTextureFile(const Arguments& args, std::string path, bool makeRelative = true);

protected:
	glm::mat4 model = glm::mat4(1);
	glm::mat4 childModel = glm::mat4(1); // Model matrix that is used as the base of to this object's children's model matricies
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
	GLuint VB;
	GLuint IB;
	GLuint tex = -1;

	// Physics rigidbody
	rp3d::RigidBody* rigidBody = nullptr;
	rp3d::Collider* collider = nullptr;

	Object* parent;
	std::vector<Object*> children;
};

// Objects which do nothing when initialized and link their model matrix to their parent each frame
// (used to hold data for models with multiple materials/submeshes)
class Submesh: public Object {
public:
	using Object::Object;

	// Submesh initialization doesn't do anything
	bool InitializeGraphics(const Arguments& args, std::string filepath = "", std::string texturePath = "invalid.png") override { return true; }
	// A submesh syncs it model matrix to its parent every frame
	void Update(unsigned int dt) override { setModelRelativeToParent(glm::mat4(1)); }
};

#endif /* OBJECT_H */
