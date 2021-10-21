#include "physics.h"
#include "engine.h"
#include "object.h"

// Backing/access for the singleton
Physics* Physics::singleton;
Physics* Physics::getSingleton() { return singleton; }

Physics::Physics(Object*& sceneRoot) : sceneRoot(sceneRoot) { }

Physics::~Physics() { }

bool Physics::Initialize(Engine* engine, const Arguments& args) {
	// Setup the singleton
	singleton = this;

	// Create the physics world
	world = factory.createPhysicsWorld();

	return true;
}

void Physics::Update(unsigned int dt) {
	// Update the physics simulation
	world->update(dt * 0.001);
}
