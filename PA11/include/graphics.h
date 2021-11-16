#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>
#include <vector>


#include "graphics_headers.h"
#include "shader.h"
#include "object.h"
#include "light.h"
#include "gui.h"
#include "arguments.h"

// Forward declarations
class Engine;
class Celestial;
class Skybox;
class Camera;

// Class which provides the graphics engine
class Graphics {
public:
	Graphics(Object::ptr& sceneRoot);
	~Graphics();
	bool initialize(int width, int height, Engine* engine, const Arguments& args);
	void update(float dt);
	void render();

	GUI* getGUI() const { return gui; }
	Camera* getCamera() const { return camera; }

	bool useFragShader = true;
protected:
	std::string errorString(GLenum error);

	GUI* gui;

	Camera *camera;
	Shader* perFragShader, *perVertShader;
	Skybox* skybox;

	GLint projectionMatrix;
	GLint viewMatrix;
	GLint modelMatrix;

	Object::ptr& sceneRoot;
};

#endif /* GRAPHICS_H */
