#include "shader.h"

#include <fstream>

Shader::Shader() {
	shaderProg = 0;
}

Shader::~Shader() {
	// Delete all of the compiled shader objects
	for (std::vector<GLuint>::iterator it = shaderObjList.begin() ; it != shaderObjList.end() ; it++)
		glDeleteShader(*it);

	// Delete the shader program
	if (shaderProg != 0) {
		glDeleteProgram(shaderProg);
		shaderProg = 0;
	}
}

bool Shader::initialize() {
	shaderProg = glCreateProgram();

	if (shaderProg == 0) {
		std::cerr << "Error creating shader program\n";
		return false;
	}

	return true;
}

// Use this method to add shaders to the program. When finished - call finalize()
bool Shader::addShader(GLenum ShaderType, std::string filePath, const Arguments& args) {
	// If the filepath doesn't already have the shader directory path, add the shader dirrectory path
	std::string shaderDirectory = args.getResourcePath() + "shaders/";
	if(filePath.find(shaderDirectory) == std::string::npos)
		filePath = shaderDirectory + filePath;

	// Make sure that the provided file exists
	std::ifstream file(filePath);
	if(!file) {
		std::cerr << "The file `" << filePath << "` doesn't exist!" << std::endl;
		return false;
	}

	// Read it in
	std::string s( (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>() );
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (ShaderObj == 0) {
		std::cerr << "Error creating shader type " << ShaderType << std::endl;
		return false;
	}

	// Save the shader object - will be deleted in the destructor
	shaderObjList.push_back(ShaderObj);

	// Set the shader's source code and compile it
	const GLchar* p[1];
	p[0] = s.c_str();
	GLint Lengths[1] = { (GLint)s.size() };
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);

	// Maje sure compilation was successful
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling: " << InfoLog << std::endl;
		return false;
	}

	// Attach the shader to our compiled program
	glAttachShader(shaderProg, ShaderObj);
	return true;
}


// After all the shaders have been added to the program call this function
// to link and validate the program.
bool Shader::finalize() {
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	// Link the program
	glLinkProgram(shaderProg);
	// Ensure program linking was successful
	glGetProgramiv(shaderProg, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		return false;
	}

	// Validate the linked program
	glValidateProgram(shaderProg);
	glGetProgramiv(shaderProg, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		return false;
	}

	// Delete the intermediate shader objects that have been added to the program
	for (std::vector<GLuint>::iterator it = shaderObjList.begin(); it != shaderObjList.end(); it++)
		glDeleteShader(*it);

	shaderObjList.clear();

	return true;
}

void Shader::enable() { glUseProgram(shaderProg); }

GLint Shader::getUniformLocation(const char* pUniformName) {
	// Get the shader location
	GLuint Location = glGetUniformLocation(shaderProg, pUniformName);

	// Warn if the location was invalid
	if (Location == INVALID_UNIFORM_LOCATION)
		fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);

	return Location;
}
