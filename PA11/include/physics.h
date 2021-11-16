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

#endif /* PHYSICS_H */
