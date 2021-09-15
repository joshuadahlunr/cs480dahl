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

class Engine;
class Planet;

class Graphics {
public:
	Graphics();
	~Graphics();
	bool Initialize(int width, int height, Engine* engine, const Arguments& args);
	void Update(unsigned int dt);
	void Render();

	GUI* getGUI() const { return m_gui; }

	template<class T = Object>
	T* getSelected() const { return dynamic_cast<T*>(selected); }

private:
	std::string ErrorString(GLenum error);

	GUI* m_gui;

	Camera *m_camera;
	Shader *m_shader;

	GLint m_projectionMatrix;
	GLint m_viewMatrix;
	GLint m_modelMatrix;

	Planet* sceneRoot;

	Object* selected;
};

#endif /* GRAPHICS_H */
