#ifndef PHYSICS_H
#define PHYSICS_H

#include "arguments.h"
#include <reactphysics3d/reactphysics3d.h>

// Forward declarations
class Engine;
class Object;

// Class which provides the physics engine
class Physics {
public:
	// Gets the singleton
	static Physics* getSingleton();

public:
	Physics(Object*& sceneRoot);
	~Physics();
	bool Initialize(Engine* engine, const Arguments& args);
	void Update(unsigned int dt);

	// Get the factory object
	rp3d::PhysicsCommon& getFactory() { return factory; }
	// Get the world
	rp3d::PhysicsWorld& getWorld() { return *world; }

protected:
	// Singleton
	static Physics* singleton;

protected:
	Object*& sceneRoot;

	// Factory object
	rp3d::PhysicsCommon factory;
	// Scene's world
	rp3d::PhysicsWorld* world;
};

#endif /* PHYSICS_H */
