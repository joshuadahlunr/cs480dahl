#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include "graphics_headers.h"
#include "arguments.h"
#include "camera.h"

class Shader;

// Class which renders a skybox
class Skybox {
public:
	Skybox();
	~Skybox();
	virtual bool Initialize(const Arguments& args, Camera* camera);
	virtual void Render();

	// Gets the camera's view matrix with translation removed
	glm::mat4 GetViewTranslateless() { return glm::mat4(glm::mat3( camera->GetView() )); }

protected:
	// Loads a cubemap
	bool LoadCubeTexture(const Arguments& args, std::string path, std::string extension = ".png"); // Loads a cubemap from a folder containing the six images with standardized names
	bool LoadCubeTexture(const Arguments& args, const std::vector<std::string>& paths); // Loads a cubmap from a list of images, expects images to provided in this order: Right, Left, Top, Bottom, Front, Back

protected:
	// List of vertecies
	std::vector<glm::vec3> Vertices;
	// Vertex buffer handle
	GLuint VB;
	// Texture handle
	GLuint tex = -1;

	// Pointer to the scene's camera
	Camera* camera;
	// Pointer to the skybox shader
	Shader* shader;

	// Locations of the projection and view matricies in the shader
	GLint projectionMatLocation;
	GLint viewMatLocation;
};

#endif /* end of include guard: SKYBOX_H */
