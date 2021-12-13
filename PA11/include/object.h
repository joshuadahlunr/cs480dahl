#ifndef OBJECT_LOCAL_H
#define OBJECT_LOCAL_H

#include <vector>
#include <memory>
#include <SDL2/SDL.h>
#include <glm/gtx/matrix_decompose.hpp> // Matrix decomposition
#include "physics.h"
#include "graphics_headers.h"
#include "arguments.h"
#include "defs.h"

#define RELATIVE_TO_PARENT true
#define CONVEX_MESH 32
#define CONCAVE_MESH 0

// Base class for objects in the scene tree, provides basic mouse, keyboard, and
// tick event propagation, along with managing the model, texture, and position in the scene tree of the object
class Object : public std::enable_shared_from_this<Object> {
public:
	using ptr = std::shared_ptr<Object>;

	Object();
	~Object();
	virtual bool initializeGraphics(const Arguments& args, std::string filepath = "", std::string texturePath = "", bool inThread = false);
	virtual bool initializePhysics(const Arguments& args, Physics& physics, int collisionGroup = CollisionGroups::CG_NONE, float mass = 1, bool addToWorldAutomatically = true);
	void addToPhysicsWorld(Physics& physics, int collisionGroup = CollisionGroups::CG_NONE);
	virtual void update(float dt);
	virtual void render(Shader* boundShader);

	// Mouse and Keyboard event propagation
	virtual void keyboard(const SDL_KeyboardEvent& e);
	virtual void mouseButton(const SDL_MouseButtonEvent& e);

	// Physics functions
	bool isPhysicsInitalized() { return rigidBody != nullptr; }
	btRigidBody& getRigidBody() { return *rigidBody; }
	void makeDynamic(bool recursive = true);
	void makeStatic(bool recursive = true);
	void makeKinematic(bool recursive = true);
	void applyForce(glm::vec3 force){ if(rigidBody) rigidBody->applyCentralForce( toBullet(force) ); } 	// Applies force to center of mass
	void applyForceAtLocalPosition(glm::vec3 force, glm::vec3 point) { if(rigidBody) rigidBody->applyForce( toBullet(force), toBullet(point) ); }
	void applyForceAtWorldPosition(glm::vec3 force, glm::vec3 point) { if(rigidBody) rigidBody->applyForce( toBullet(force), toBullet(point - getPosition()) ); }
	void applyTorque(glm::vec3 torque) { if(rigidBody) rigidBody->applyTorque( toBullet(torque) ); }
	// void addCollisionCallback(Physics::ContactEvent event);// { if(Physics::getSingleton()) Physics::getSingleton()->addContactCallback(shared_from_this(), event); }

	// Physics Collider adding functions
	void createCapsuleCollider(float radius, float height);
	void createBoxCollider(glm::vec3 halfExtents);
	void createSphereCollider(float radius);
	virtual bool createMeshCollider(const Arguments& args, Physics& physics, size_t maxHulls = CONVEX_MESH, std::string path = "");

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
	void resetOrientation() { setRotation(glm::quat()); }
	void setRotation(glm::quat rot, bool relativeToParent = false);
	glm::quat getRotation();
	glm::vec3 up() {return glm::vec3(model[0][1], model[1][1], model[2][1]);}
	glm::vec3 down() {return -glm::vec3(-model[0][1], model[1][1], -model[2][1]);}
	glm::vec3 right() {return glm::vec3(model[0][0], model[1][0], model[2][0]);}
	glm::vec3 forward() {return glm::vec3(model[0][2], model[1][2], model[2][2]);}
	void rotate(float rads, glm::vec3 axis) { setModel(glm::rotate(model, rads, axis)); }
	void rotate(glm::quat rotation) { setModel(glm::mat4_cast(rotation) * model); }
	void setScale(glm::vec3 scale, bool relativeToParent = false);
	glm::vec3 getScale();
	void scale(glm::vec3 scale) { setModel(glm::scale(model, scale)); }
	void setLinearVelocity(glm::vec3 velocity){ if(rigidBody) rigidBody->setLinearVelocity( toBullet(velocity) ); } // TODO: Does linear velocity need to propagate through the scene tree?
	glm::vec3 getLinearVelocity(){
		if(rigidBody) return toGLM( rigidBody->getLinearVelocity() );
		return glm::vec3(0);
	}
	void setAngularVelocity(glm::vec3 velocity){ if(rigidBody) rigidBody->setAngularVelocity( toBullet(velocity) ); }
	glm::vec3 getAngularVelocity(){
		if(rigidBody) return toGLM( rigidBody->getAngularVelocity() );
		return glm::vec3(0);
	}

	// The depth in the scene tree of this object
	const uint sceneDepth = 0;

	// Load a texture from a file
	bool loadTextureFile(const Arguments& args, std::string path, bool makeRelative = true);
	// Use the same texture as another already loaded object
	void linkTexture(Object::ptr object) { tex = object->tex; }

	// Uploads the model data to the GPU
	void finalizeModel(bool recursive = true);

protected:
	// Model/Texture loading
	bool LoadModelFile(const Arguments& args, const std::string& path, glm::mat4 onImportTransformation = glm::mat4(1), bool inThread = true);

	// Create a reference to the invalid texture
	bool initalizeInvalidTexture(const Arguments& args);

	// Physics functions
	void setPhysicsTransform(btTransform&& t) {
		if(rigidBody) { 
			rigidBody->setWorldTransform(t);
			rigidBody->activate(); // Make sure the body is awake and checking for collisions when we move it
		}
	}
	void syncPhysicsWithGraphics(){ setPhysicsTransform( toBullet(getModel()) ); }
	void syncGraphicsWithPhysics(){ if(rigidBody) setModel( toGLM(rigidBody->getWorldTransform()) ); }
		

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

	// std::unique_ptr<ConcaveCollisionMesh>& getConcaveCollisionMesh(){
	// 	if(collisionMesh->type != CollisionMesh::Type::Concave) 
	// 		throw std::runtime_error("Attempted to convert a convex collision mesh to a concave collision mesh");
		
	// 	return *(std::unique_ptr<ConcaveCollisionMesh>*) &collisionMesh;
	// }

	// std::unique_ptr<ConvexCollisionMesh>& getConvexCollisionMesh(){
	// 	if(collisionMesh->type != CollisionMesh::Type::Convex) 
	// 		throw std::runtime_error("Attempted to convert a concave collision mesh to a convex collision mesh");

	// 	return *(std::unique_ptr<ConvexCollisionMesh>*) &collisionMesh;
	// }

protected:
	glm::mat4 model = glm::mat4(1);
	glm::mat4 childModel = glm::mat4(1); // Model matrix that is used as the base of to this object's children's model matricies
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	GLuint VB = -1;
	GLuint IB = -1;
	GLuint tex = -1;
	static GLuint invalidTex;

	// Physics rigidbody
	bool addedToPhysicsWorld = false;
	std::vector<std::unique_ptr<btTriangleMesh>> trimeshs;
	std::vector<std::unique_ptr<btConvexTriangleMeshShape>> shapes;
	std::unique_ptr<btDefaultMotionState> motionState = nullptr;
	std::unique_ptr<btRigidBody> rigidBody = nullptr;
	std::unique_ptr<btCollisionShape> collisionShape = nullptr;

	// rp3d::Collider* collider = nullptr;

	Object* parent;
	std::vector<Object::ptr> children;
};

// Objects which do nothing when initialized and link their model matrix to their parent each frame
// (used to hold data for models with multiple materials/submeshes)
class Submesh: public Object {
public:
	using Object::Object;

	// Submesh initialization doesn't do anything
	bool initializeGraphics(const Arguments& args, std::string filepath = "", std::string texturePath = "invalid.png", bool autoUpload = true) override { return true; }
	// A submesh syncs it model matrix to its parent every frame
	void update(float dt) override { setModelRelativeToParent(glm::mat4(1)); }
};

#endif /* OBJECT_H */
