#include "skybox.h"

#include <fstream>
#include <sstream>

#include "shader.h"

// Texture loading
#include "stb_image.h"

Skybox::Skybox() {
	// Create the vertex and face buffers for this object
	glGenBuffers(1, &VB);
}

Skybox::~Skybox() {
	// Clean up the lists of vertecies and indices
	Vertices.clear();
}

bool Skybox::Initialize(const Arguments& args, Camera* camera){
	bool success = true;

	// Assign the camera
	this->camera = camera;

	// Create a cube the skybox will be draw onto
	Vertices = {
		{-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f},

        {-1.0f, -1.0f,  1.0f},
        {-1.0f, -1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f,  1.0f},
        {-1.0f, -1.0f,  1.0f},

        { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},

        {-1.0f, -1.0f,  1.0f},
        {-1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f},
        {-1.0f, -1.0f,  1.0f},

        {-1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f, -1.0f},

        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f,  1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f}
	};

	// Upload the cube to the gpu
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// Create a shader program for the skybox
	shader = new Shader();
	success &= shader->Initialize();
	success &= shader->AddShader(GL_VERTEX_SHADER, "skybox.vert.glsl", args);
	success &= shader->AddShader(GL_FRAGMENT_SHADER, "skybox.frag.glsl", args);
	success &= shader->Finalize();

	// Find the projection matrix in the skybox shader
	projectionMatLocation = shader->GetUniformLocation("projectionMat");
	if(projectionMatLocation == INVALID_UNIFORM_LOCATION){
		std::cerr << "Can't find uniform `projectionMat` in skybox shader" << std::endl;
		success = false;
	}

	// Find the view matrix in the skybox shader
	viewMatLocation = shader->GetUniformLocation("viewMat");
	if(viewMatLocation == INVALID_UNIFORM_LOCATION){
		std::cerr << "Can't find uniform `viewMat` in skybox shader" << std::endl;
		success = false;
	}

	// Load the cube map
	success &= LoadCubeTexture(args, args.getResourcePath() + "textures/skyboxs/clouds");

	return success;
}

// Loads a cubemap from a folder containing the six images with standardized names
bool Skybox::LoadCubeTexture(const Arguments& args, std::string path, std::string extension){
	// Create the six paths based on standardized names
	std::vector<std::string> paths {
		path + "/right" + extension,
		path + "/left" + extension,
		path + "/top" + extension,
		path + "/bottom" + extension,
		path + "/front" + extension,
		path + "/back" + extension
	};

	// Load the cubemao
	return LoadCubeTexture(args, paths);
}

// Loads a cubmap from a list of images, expects images to provided in this order: Right, Left, Top, Bottom, Front, Back
bool Skybox::LoadCubeTexture(const Arguments& args, const std::vector<std::string>& paths) {
	// Complain if we aren't given 6 images
	if(paths.size() != 6){
		std::cerr << "Cubemaps must be initialized with 6 images." << std::endl;
		return false;
	}

	// Create texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	// Load the images
	int width, height, channelsPresent;
	for (unsigned int i = 0; i < paths.size(); i++){
        unsigned char *img = stbi_load(paths[i].c_str(), &width, &height, &channelsPresent, 3);
        if (img){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
            stbi_image_free(img);
        } else {
            std::cerr << "Failed to load cubemap texture `" << paths[i] << "`" << std::endl;
            stbi_image_free(img);
			return false;
        }
    }

	// Set parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return true;
}

void Skybox::Render() {
	// Disable depth testing
	glDepthMask(GL_FALSE);

	// Enable the skybox shader
	shader->Enable();

	// Set the view and projection matrix
	glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, glm::value_ptr(GetViewTranslateless()));
	glUniformMatrix4fv(projectionMatLocation, 1, GL_FALSE, glm::value_ptr(camera->GetProjection()));

	// Enable 1 vertex attribute
	glEnableVertexAttribArray(0);

	// Specify that we are using the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	// Specify where in the vertex buffer we can find position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

	// Bind texture (if it exists)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	// Disable backface culling
	glDisable(GL_CULL_FACE);
	// Draw the triangles
	glDrawArrays(GL_TRIANGLES, 0, Vertices.size());

	// Disable the attributes
	glDisableVertexAttribArray(0);
	// Enable backface culling
	glEnable(GL_CULL_FACE);
	// Enable depth testing
	glDepthMask(GL_TRUE);
}
