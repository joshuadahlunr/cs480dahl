#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <string>
using namespace std;

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "object.h"
#include "arguments.h"

class Graphics {
public:
	Graphics();
	~Graphics();
	bool Initialize(int width, int height, const Arguments& args);
	void Update(unsigned int dt);
	void Render();

private:
	std::string ErrorString(GLenum error);

	Camera *m_camera;
	Shader *m_shader;

	GLint m_projectionMatrix;
	GLint m_viewMatrix;
	GLint m_modelMatrix;

	Object *m_cube;
};

#endif /* GRAPHICS_H */
