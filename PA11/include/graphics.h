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


// The height and width of our shadow maps
#define SHADOW_RESOLUTION 4096


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
	void renderScene(Shader* boundShader);

	GUI* getGUI() const { return gui; }
	Camera* getCamera() const { return camera; }

	bool useFragShader = true;
protected:
	std::string errorString(GLenum error);

	GUI* gui;

	Engine* engine;
	Camera* camera;
	Shader* perFragShader, *perVertShader, *depthShader;
	Shader* debug;
	// Skybox* skybox;

	GLint projectionMatrix;
	GLint viewMatrix;
	GLint modelMatrix;

	// Shadow Mapping
	GLuint depthMapFBO;
	GLuint depthMap;
	GLuint debugVBO;
	GLint lightSpaceMatrixLocation;

	Object::ptr& sceneRoot;
};

#endif /* GRAPHICS_H */
