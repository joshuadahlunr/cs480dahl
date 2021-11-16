#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <string>

#include "graphics_headers.h"
#include "arguments.h"

// Object representing a shader program
class Shader {
public:
	Shader();
	~Shader();
	bool initialize();
	void enable();
	bool addShader(GLenum ShaderType, std::string filePath, const Arguments& args);
	bool finalize();
	GLint getUniformLocation(const char* pUniformName);

private:
	GLuint shaderProg;
	std::vector<GLuint> shaderObjList;
};

#endif  /* SHADER_H */
