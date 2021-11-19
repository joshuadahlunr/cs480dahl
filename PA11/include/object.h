#ifndef OBJECT_LOCAL_H
#define OBJECT_LOCAL_H

#include <vector>
#include <memory>
#include <SDL2/SDL.h>
#include <glm/gtx/matrix_decompose.hpp> // Matrix decomposition
#include "physics.h"
#include "graphics_headers.h"
#include "arguments.h"

#define RELATIVE_TO_PARENT true

// Base class for objects in the scene tree, provides basic mouse, keyboard, and
// tick event propagation, along with managing the model, texture, and position in the scene tree of the object
class Object : public std::enable_shared_from_this<Object> {
public:
	using ptr = std::shared_ptr<Object>;

	Object();
	~Object();
	virtual bool initializeGraphics(const Arguments& args, std::string filepath = "", std::string texturePath = "invalid.png");
	virtual bool initializePhysics(const Arguments& args, Physics& physics, bool _static);
	virtual void update(float dt);
	virtual void render(Shader* boundShader);

	// Mouse and Keyboard event propagation
	virtual void keyboard(const SDL_KeyboardEvent& e);
	virtual void mouseButton(const SDL_MouseButtonEvent& e);

	// Physics functions
	rp3d::RigidBody& getRigidBody() { return *rigidBody; }
	rp3d::Collider& getCollider() { return *collider; }
	void applyForce(glm::vec3 force) { if(rigidBody) rigidBody->applyForceToCenterOfMass( toReact(force) ); } 	// Applies force to center of mass
	void applyForceAtLocalPosition(glm::vec3 force, glm::vec3 point) { if(rigidBody) rigidBody->applyForceAtLocalPosition( toReact(force), toReact(point) ); }
	void applyForceAtWorldPosition(glm::vec3 force, glm::vec3 point) { if(rigidBody) rigidBody->applyForceAtWorldPosition( toReact(force), toReact(point) ); }
	void applyTorque(glm::vec3 torque) { if(rigidBody) rigidBody->applyTorque( toReact(torque) ); }
	void addCollisionCallback(Physics::ContactEvent event) { if(Physics::getSingleton()) Physics::getSingleton()->addContactCallback(shared_from_this(), event); }

	// Physics Collider adding functions
	void addCapsuleCollider(float radius, float height, glm::vec3 translation = glm::vec3(0), glm::quat rotation = glm::quat_identity());
	void addBoxCollider(glm::vec3 halfExtents, glm::vec3 translation = glm::vec3(0), glm::quat rotation = glm::quat_identity());
	void addSphereCollider(float radius, glm::vec3 translation = glm::vec3(0), glm::quat rotation = glm::quat_identity());
	bool addMeshCollider(const Arguments& args, bool makeConvex = true, std::string path = "", glm::vec3 translation = glm::vec3(0), glm::quat rotation = glm::quat_identity());

	// Scene tree management
	Object::ptr setParent(Object::ptr p);
	Object::ptr getParent() const { return parent->shared_from_this(); }
	Object::ptr addChild(Object::ptr child);
	const std::vector<Object::ptr>& getChildren() const { return children; }

	// Sets model matrix
	glm::mat4 getModel() { return model; }
	glm::mat4 getChildBaseModel() { return childModel;  }
	void setModel(glm::mat4 _model);
	void setChildModel(glm::mat4 _model) { childModel = _model; }
	void setModelRelativeToParent(glm::mat4 _model);
	void setChildModelRelativeToParent(glm::mat4 _model);

	// Set/get position, rotation, scale, and velocity
	void setPosition(glm::vec3 _pos, bool relativeToParent = false);
	glm::vec3 getPosition() { return glm::vec3(model[3][0], model[3][1], model[3][2]); }
	void translate(glm::vec3 translation) { setModel(glm::translate(model, translation)); }
	void setRotation(glm::quat rot, bool relativeToParent = false);
	glm::quat getRotation();
	void rotate(float rads, glm::vec3 axis) { setModel(glm::rotate(model, rads, axis)); }
	void rotate(glm::quat rotation) { setModel(glm::mat4_cast(rotation) * model); }
	void setScale(glm::vec3 scale, bool relativeToParent = false);
	glm::vec3 getScale();
	void scale(glm::vec3 scale) { setModel(glm::scale(model, scale)); }
	void setLinearVelocity(glm::vec3 velocity){ if(rigidBody) rigidBody->setLinearVelocity( toReact(velocity) ); } // TODO: Does linear velocity need to propagate through the scene tree?
	glm::vec3 getLinearVelocity(){
		if(rigidBody) return toGLM( rigidBody->getLinearVelocity() );
		return glm::vec3(0);
	}
	void setAngularVelocity(glm::vec3 velocity){ if(rigidBody) rigidBody->setAngularVelocity( toReact(velocity) ); }
	glm::vec3 getAngularVelocity(){
		if(rigidBody) return toGLM( rigidBody->getAngularVelocity() );
		return glm::vec3(0);
	}

	// The depth in the scene tree of this object
	const uint sceneDepth = 0;
	bool loadTextureFile(const Arguments& args, std::string path, bool makeRelative = true);

protected:
	// Uploads the model data to the GPU
	void finalizeModel();

	// Model/Texture loading
	bool LoadModelFile(const Arguments& args, const std::string& path, glm::mat4 onImportTransformation = glm::mat4(1));

	// Physics functions
	void setPhysicsTransform(rp3d::Transform t) { if(rigidBody) rigidBody->setTransform(t); }
	void syncPhysicsWithGraphics(){ setPhysicsTransform( toReact(getModel()) ); }
	void syncGraphicsWithPhysics() { if(rigidBody) setModel( toGLM(rigidBody->getTransform()) ); }

	// Decompose the model matrix
	void decomposeModelMatrix(glm::vec3& translate, glm::quat& rotate, glm::vec3& scale){
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(model, scale, rotate, translate, skew, perspective);
	}

	// Decompose the child's base matrix
	void decomposeChildBaseModelMatrix(glm::vec3& translate, glm::quat& rotate, glm::vec3& scale){
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(childModel, scale, rotate, translate, skew, perspective);
	}

protected:
	glm::mat4 model = glm::mat4(1);
	glm::mat4 childModel = glm::mat4(1); // Model matrix that is used as the base of to this object's children's model matricies
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	CollisionMesh collisionMesh;

	GLuint VB;
	GLuint IB;
	GLuint tex = -1;

	// Physics rigidbody
	rp3d::RigidBody* rigidBody = nullptr;
	rp3d::Collider* collider = nullptr;

	Object* parent;
	std::vector<Object::ptr> children;
};

// Objects which do nothing when initialized and link their model matrix to their parent each frame
// (used to hold data for models with multiple materials/submeshes)
class Submesh: public Object {
public:
	using Object::Object;

	// Submesh initialization doesn't do anything
	bool initializeGraphics(const Arguments& args, std::string filepath = "", std::string texturePath = "invalid.png") override { return true; }
	// A submesh syncs it model matrix to its parent every frame
	void update(float dt) override { setModelRelativeToParent(glm::mat4(1)); }
};

#endif /* OBJECT_H */
