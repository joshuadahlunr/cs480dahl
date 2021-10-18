#ifndef PHYSICS_H
#define PHYSICS_H

#include "object.h"
#include "arguments.h"

// Forward declarations
class Engine;

// Class which provides the physics engine
class Physics {
public:
	Physics(Object*& sceneRoot);
	~Physics();
	bool Initialize(Engine* engine, const Arguments& args);
	void Update(unsigned int dt);

private:
	Object*& sceneRoot;
};

#endif /* PHYSICS_H */
