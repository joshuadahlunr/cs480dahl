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
	bool Initialize();
	void Enable();
	bool AddShader(GLenum ShaderType, std::string filePath, const Arguments& args);
	bool Finalize();
	GLint GetUniformLocation(const char* pUniformName);

private:
	GLuint m_shaderProg;
	std::vector<GLuint> m_shaderObjList;
};

#endif  /* SHADER_H */
