#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>
using namespace std;

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "object.h"
#include "gui.h"
#include "arguments.h"

// Forward declarations
class Engine;
class Celestial;
class Skybox;

// Class which provides the graphics engine
class Graphics {
public:
	Graphics();
	~Graphics();
	bool Initialize(int width, int height, Engine* engine, const Arguments& args);

	// Recursively initializes a scene tree from the provided json data
	Celestial* CelestialFromJson(const Arguments& args, json j, uint depth = 0);

	void Update(unsigned int dt);
	void Render();

	GUI* getGUI() const { return m_gui; }

private:
	std::string ErrorString(GLenum error);

	GUI* m_gui;

	Camera *m_camera;
	Shader *m_shader;
	Skybox* m_skybox;

	GLint m_projectionMatrix;
	GLint m_viewMatrix;
	GLint m_modelMatrix;

	Object* sceneRoot;
};

#endif /* GRAPHICS_H */
