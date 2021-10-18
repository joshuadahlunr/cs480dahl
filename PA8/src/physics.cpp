#include "physics.h"
#include "engine.h"

Physics::Physics(Object*& sceneRoot) : sceneRoot(sceneRoot) { }

Physics::~Physics() { }

bool Physics::Initialize(Engine* engine, const Arguments& args) {
	// Stub
	return true;
}

void Physics::Update(unsigned int dt) {
	// Stub
}
