#include "graphics.h"
#include "application.h"
#include "camera.h"

#include "skybox.h"
#include <fstream>

Graphics::Graphics(Object*& sceneRoot) : sceneRoot(sceneRoot) { }

Graphics::~Graphics() { }

bool Graphics::Initialize(int width, int height, Engine* engine, const Arguments& args) {
	// Used for the linux OS
#if !defined(__APPLE__) && !defined(MACOSX)
	// cout << glewGetString(GLEW_VERSION) << endl;
	glewExperimental = GL_TRUE;

	auto status = glewInit();

	// This is here to grab the error that comes from glew init.
	// This error is an GL_INVALID_ENUM that has no effects on the performance
	glGetError();

	//Check for error
	if (status != GLEW_OK) {
		std::cerr << "GLEW Error: " << glewGetErrorString(status) << "\n";
		return false;
	}
#endif

	// For OpenGL 3
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Init Camera
	m_camera = new Camera((Application*) engine);
	if(!m_camera->Initialize(width, height)) {
		printf("Camera Failed to Initialize\n");
		return false;
	}

	// Hookup camera mouse and keyboard events
	engine->keyboardEvent += [&](auto event) { m_camera->Keyboard(event); };
	engine->mouseButtonEvent += [&](auto event) { m_camera->MouseButton(event); };
	engine->mouseMotionEvent += [&](auto event) { m_camera->MouseMotion(event); };
	engine->mouseWheelEvent += [&](auto event) { m_camera->MouseWheel(event); };

	// Set up the shaders
	perVertShader = new Shader();
	if(!perVertShader->Initialize()) {
		printf("Shader Failed to Initialize\n");
		return false;
	}

	// Add the vertex shader
	if(!perVertShader->AddShader(GL_VERTEX_SHADER, args.getPerVertexVertexFilePath(), args)) {
		printf("Per Vertex, Vertex Shader failed to Initialize\n");
		return false;
	}

	// Add the fragment shader
	if(!perVertShader->AddShader(GL_FRAGMENT_SHADER, args.getPerVertexFragmentFilePath(), args)) {
		printf("Per Vertex, Fragment Shader failed to Initialize\n");
		return false;
	}

	// Link the program
	if(!perVertShader->Finalize()) {
		printf("Program failed to Finalize\n");
		return false;
	}

	// Set up the shaders
	perFragShader = new Shader();
	if(!perFragShader->Initialize()) {
		printf("Shader Failed to Initialize\n");
		return false;
	}

	// Add the vertex shader
	if(!perFragShader->AddShader(GL_VERTEX_SHADER, args.getPerFragmentVertexFilePath(), args)) {
		printf("Per Fragment, Vertex Shader failed to Initialize\n");
		return false;
	}

	// Add the fragment shader
	if(!perFragShader->AddShader(GL_FRAGMENT_SHADER, args.getPerFragmentFragmentFilePath(), args)) {
		printf("Per Fragment, Fragment Shader failed to Initialize\n");
		return false;
	}

	// Link the program
	if(!perFragShader->Finalize()) {
		printf("Program failed to Finalize\n");
		return false;
	}

	// Create the GUI
	m_gui = new GUI();
	if(!m_gui->Initialize(engine)) {
		printf("GUI Failed to Initialize\n");
		return false;
	}

	// Create the skybox
	m_skybox = new Skybox();
	if(!m_skybox->Initialize(args, m_camera)) {
		printf("Skybox Failed to Initialize\n");
		return false;
	}

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return true;
}

void Graphics::Update(unsigned int dt) {
	// Update the camera
	m_camera->Update(dt);
}

void Graphics::Render() {
	//clear the screen
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render the skybox first (thus everything else is drawn in front of it)
	m_skybox->Render();

	Shader* boundShader;

	// Start the correct program
	if(useFragShader){
		perFragShader->Enable();
		boundShader = perFragShader;
	} else {
		perVertShader->Enable();
		boundShader = perVertShader;
	}

	// Send in the projection and view to the shader
	glUniformMatrix4fv(boundShader->GetUniformLocation("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
	glUniformMatrix4fv(boundShader->GetUniformLocation("viewMatrix"), 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));

	// Set all objects lighting materials
	glm::vec4 materialAmbient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	glm::vec4 materialDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 materialSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float materialShininess = 50.0f;
	glUniform4fv(boundShader->GetUniformLocation("material.ambient"), 1, glm::value_ptr(materialAmbient));
	glUniform4fv(boundShader->GetUniformLocation("material.diffuse"), 1, glm::value_ptr(materialDiffuse));
	glUniform4fv(boundShader->GetUniformLocation("material.specular"), 1, glm::value_ptr(materialSpecular));
	glUniform1f(boundShader->GetUniformLocation("material.shininess"), materialShininess);

	glUniform1ui(boundShader->GetUniformLocation("num_lights"), Light::count);

	// Render the object
	sceneRoot->Render(boundShader);

	// Render the GUI
	m_gui->Render();

	// Get any errors from OpenGL
	auto error = glGetError();
	if ( error != GL_NO_ERROR ) {
		string val = ErrorString( error );
		std::cout<< "Error initializing OpenGL! " << error << ", " << val << std::endl;
	}
}

std::string Graphics::ErrorString(GLenum error) {
	if(error == GL_INVALID_ENUM)
		return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
	else if(error == GL_INVALID_VALUE)
		return "GL_INVALID_VALUE: A numeric argument is out of range.";
	else if(error == GL_INVALID_OPERATION)
		return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
	else if(error == GL_INVALID_FRAMEBUFFER_OPERATION)
		return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
	else if(error == GL_OUT_OF_MEMORY)
		return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
	else
		return "None";
}
