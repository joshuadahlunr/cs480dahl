#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;

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
	Graphics(Object*& sceneRoot);
	~Graphics();
	bool Initialize(int width, int height, Engine* engine, const Arguments& args);
	void Update(float dt);
	void Render();

	GUI* getGUI() const { return m_gui; }
	Camera* getCamera() const { return m_camera; }

	bool useFragShader = true;
protected:
	std::string ErrorString(GLenum error);

	GUI* m_gui;

	Camera *m_camera;
	Shader* perFragShader, *perVertShader;
	Skybox* m_skybox;

	GLint m_projectionMatrix;
	GLint m_viewMatrix;
	GLint m_modelMatrix;


	Object*& sceneRoot;
};

#endif /* GRAPHICS_H */
