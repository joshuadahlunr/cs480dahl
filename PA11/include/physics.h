#ifndef PHYSICS_H
#define PHYSICS_H

#include "arguments.h"
#include <reactphysics3d/reactphysics3d.h>
#include "graphics_headers.h"
#include <map>
#include <functional>

// Uncomment to enable physics debug rendering
// #define PHYSICS_DEBUG

// Forward declarations
class Engine;
class Object;
class Shader;
class Camera;

// Class which provides the physics engine
class Physics : public rp3d::EventListener {
public:
	//using ContactEvent = void (*)(const rp3d::CollisionCallback::ContactPair&);
	using ContactEvent = std::function<void(const rp3d::CollisionCallback::ContactPair&)>;

	// Gets the singleton
	static Physics* getSingleton();

public:
	Physics(Object*& sceneRoot);
	~Physics();
	bool initialize(Engine* engine, const Arguments& args);
	void update(float dt);
#ifdef PHYSICS_DEBUG
	void render(Camera* camera);
#endif

	virtual void onContact(const rp3d::CollisionCallback::CallbackData& callbackData) override;

	// Get the factory object
	rp3d::PhysicsCommon& getFactory() { return factory; }
	// Get the world
	rp3d::PhysicsWorld& getWorld() { return *world; }

	void addContactCallback(Object* obj, ContactEvent e);

protected:
	// Singleton
	static Physics* singleton;

protected:
	Object*& sceneRoot;

	// Factory object
	rp3d::PhysicsCommon factory;
	// Scene's world
	rp3d::PhysicsWorld* world;

	std::map<uint32_t, ContactEvent> contactEvents;

#ifdef PHYSICS_DEBUG
	// Debug rendering variables
	GLuint debugLineBuffer;
	Shader* debugShader;
#endif
};


// -- React Interop --


// Convert vector3s
static rp3d::Vector3 toReact(const glm::vec3& v) { return rp3d::Vector3(v.x, v.y, v.z); }
static rp3d::Vector3 toReact(const glm::vec3&& v) { return toReact(v); }
// Convert quaternions
static rp3d::Quaternion toReact(const glm::quat& q) {
	auto rotationMat = glm::mat3_cast(q);
	float* rot = glm::value_ptr(rotationMat);
	return { rp3d::Matrix3x3(rot[0], rot[1], rot[2], rot[3], rot[4], rot[5], rot[6], rot[7], rot[8]) };
}
static rp3d::Quaternion toReact(const glm::quat&& q) { return toReact(q); }
// Convert vec3 and quaternion to transform
static rp3d::Transform toReact(const glm::vec3 pos, const glm::quat rotation){
	rp3d::Transform out;
	out.setPosition(toReact(pos));
	out.setOrientation(toReact(rotation));
	return out;
}
// Convert mat4 to transform
static rp3d::Transform toReact(const glm::mat4& mat){
	rp3d::Transform out;
	out.setFromOpenGL((rp3d::decimal*) glm::value_ptr(mat));
	return out;
}
static rp3d::Transform toReact(const glm::mat4&& mat){ return toReact(mat); }

// Convert vector3s
static glm::vec3 toGLM(const rp3d::Vector3& v) { return {v.x, v.y, v.z}; }
static glm::vec3 toGLM(const rp3d::Vector3&& v) { return toGLM(v); }
// Convert quaternions
static glm::quat toGLM(const rp3d::Quaternion& q){
	auto mat = q.getMatrix();
	return glm::quat_cast(glm::mat3(mat[0][0], mat[0][1], mat[0][2], mat[1][0], mat[1][1], mat[1][2], mat[2][0], mat[2][1], mat[2][2]));
}
static glm::quat toGLM(const rp3d::Quaternion&& q){ return toGLM(q); }
// Convert transform to mat4
static glm::mat4 toGLM(const rp3d::Transform& t){
	glm::mat4 out;
	t.getOpenGLMatrix(glm::value_ptr(out));
	return out;
}
static glm::mat4 toGLM(const rp3d::Transform&& t){ return toGLM(t); }

#endif /* PHYSICS_H */
