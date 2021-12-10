#include "graphics.h"
#include "application.h"
#include "camera.h"
#include "window.h"

#include "skybox.h"
#include <fstream>

Graphics::Graphics(Object::ptr& sceneRoot) : sceneRoot(sceneRoot) { }

Graphics::~Graphics() { }

bool Graphics::initialize(int width, int height, Engine* engine, const Arguments& args) {
	this->engine = engine;

	// Used for the linux OS
#if !defined(__APPLE__) && !defined(MACOSX)
	// std::cout << glewGetString(GLEW_VERSION) << std::endl;
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
	camera = new Camera((Application*) engine);
	if(!camera->initialize(width, height)) {
		printf("Camera Failed to initialize\n");
		return false;
	}

	// Hookup camera mouse and keyboard events
	engine->keyboardEvent += [&](auto event) { camera->keyboard(event);};
	engine->mouseButtonEvent += [&](auto event) { camera->mouseButton(event); };
	engine->mouseMotionEvent += [&](auto event) { camera->mouseMotion(event); };
	engine->mouseWheelEvent += [&](auto event) { camera->mouseWheel(event); };

	// Set up the Per Vertex shader
	perVertShader = new Shader();
	if(!perVertShader->initialize()) {
		printf("Shader Failed to initialize\n");
		return false;
	}
	// Add the vertex shader
	if(!perVertShader->addShader(GL_VERTEX_SHADER, args.getPerVertexVertexFilePath(), args)) {
		printf("Per Vertex, Vertex Shader failed to initialize\n");
		return false;
	}
	// Add the fragment shader
	if(!perVertShader->addShader(GL_FRAGMENT_SHADER, args.getPerVertexFragmentFilePath(), args)) {
		printf("Per Vertex, Fragment Shader failed to initialize\n");
		return false;
	}
	// Link the program
	if(!perVertShader->finalize()) {
		printf("Program failed to finalize\n");
		return false;
	}

	// Set up the Per-Fragment shader
	perFragShader = new Shader();
	if(!perFragShader->initialize()) {
		printf("Shader Failed to initialize\n");
		return false;
	}
	// Add the vertex shader
	if(!perFragShader->addShader(GL_VERTEX_SHADER, args.getPerFragmentVertexFilePath(), args)) {
		printf("Per Fragment, Vertex Shader failed to initialize\n");
		return false;
	}
	// Add the fragment shader
	if(!perFragShader->addShader(GL_FRAGMENT_SHADER, args.getPerFragmentFragmentFilePath(), args)) {
		printf("Per Fragment, Fragment Shader failed to initialize\n");
		return false;
	}
	// Link the program
	if(!perFragShader->finalize()) {
		printf("Program failed to finalize\n");
		return false;
	}


	// Set up the Depth shader
	depthShader = new Shader();
	if(!depthShader->initialize()) {
		printf("Shader Failed to initialize\n");
		return false;
	}
	// Add the vertex shader
	if(!depthShader->addShader(GL_VERTEX_SHADER, "depth.vert.glsl", args)) {
		printf("Depth Vertex Shader failed to initialize\n");
		return false;
	}
	// Add the fragment shader
	if(!depthShader->addShader(GL_FRAGMENT_SHADER, "empty.glsl", args)) {
		printf("Depth Fragment Shader failed to initialize\n");
		return false;
	}
	// Link the program
	if(!depthShader->finalize()) {
		printf("Program failed to finalize\n");
		return false;
	}
	lightSpaceMatrixLocation = depthShader->getUniformLocation("lightSpaceMatrix");

	// Set up the Depth shader
	debug = new Shader();
	if(!debug->initialize()) {
		printf("Shader Failed to initialize\n");
		return false;
	}
	// Add the vertex shader
	if(!debug->addShader(GL_VERTEX_SHADER, "fullscreenQuad.vert.glsl", args)) {
		printf("Debug Vertex Shader failed to initialize\n");
		return false;
	}
	// Add the fragment shader
	if(!debug->addShader(GL_FRAGMENT_SHADER, "depthDebug.frag.glsl", args)) {
		printf("Debug Fragment Shader failed to initialize\n");
		return false;
	}
	// Link the program
	if(!debug->finalize()) {
		printf("Program failed to finalize\n");
		return false;
	}
	// Vertex buffer for debug
	glGenBuffers(1, &debugVBO);


	// Create the GUI
	gui = new GUI();
	if(!gui->initialize(engine)) {
		printf("GUI Failed to initialize\n");
		return false;
	}

	// Create the skybox
	// skybox = new Skybox();
	// if(!skybox->initialize(args, camera)) {
	// 	printf("Skybox Failed to initialize\n");
	// 	return false;
	// }

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	// Shadow Mapping
	glGenFramebuffers(1, &depthMapFBO);  // Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	// Texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RESOLUTION, SHADOW_RESOLUTION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
	// Bind texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void Graphics::update(float dt) {
	// Update the camera
	camera->update(dt);
	// Update the GUI
	gui->update(dt);
}

void Graphics::render() {

	glm::mat4 lightSpaceMatrix(-1);

	// If there is a primary directional light...
	if(DirectionalLight::getPrimary()){
		// Render the scene's depth into the depth buffer
		glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, /*near plane*/1.f, /*far plane*/350.f);
			glm::mat4 lightView = glm::lookAt<float>(DirectionalLight::getPrimary()->lightDirection * -150.f, {0, 0, 0}, {0, 1, 0});
			lightSpaceMatrix = lightProjection * lightView;

			depthShader->enable();
			glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

			renderScene(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK);


		// // Depth Debug
		// glClearColor(0.0, 0.0, 0.0, 1.0);
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// // Set the shader and bind the depth map
		// debug->enable();
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, depthMap);	
		// // Draw the fullscreen quad
		// glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	
	// Then render the scene normally
	auto windowDims = engine->getWindow()->getDimensions();
	glViewport(0, 0, windowDims.x, windowDims.y);

	//clear the screen
	glClearColor(.7, .7, .7, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render the skybox first (thus everything else is drawn in front of it)
	// skybox->render();

	// Start the correct program
	Shader* boundShader;
	if(useFragShader) {
		perFragShader->enable();
		boundShader = perFragShader;
	} else {
		perVertShader->enable();
		boundShader = perVertShader;
	}

	// Bind the depth map as texture 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	// Send in the projection and view to the shader
	glUniformMatrix4fv(boundShader->getUniformLocation("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(camera->getProjection()));
	glUniformMatrix4fv(boundShader->getUniformLocation("viewMatrix"), 1, GL_FALSE, glm::value_ptr(camera->getView()));
	// Bind the lightSpaceMatrix
	glUniformMatrix4fv(boundShader->getUniformLocation("lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	// Set all objects lighting materials
	glm::vec4 materialAmbient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	glm::vec4 materialDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 materialSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float materialShininess = 50.0f;
	glUniform4fv(boundShader->getUniformLocation("material.ambient"), 1, glm::value_ptr(materialAmbient));
	glUniform4fv(boundShader->getUniformLocation("material.diffuse"), 1, glm::value_ptr(materialDiffuse));
	glUniform4fv(boundShader->getUniformLocation("material.specular"), 1, glm::value_ptr(materialSpecular));
	glUniform1f(boundShader->getUniformLocation("material.shininess"), materialShininess);

	// Bind the number of lights
	glUniform1ui(boundShader->getUniformLocation("num_lights"), Light::count);

	renderScene(boundShader);
}

void Graphics::renderScene(Shader* boundShader) {
	// Preform custom rendering
	engine->render(boundShader);

	// render the object
	sceneRoot->render(boundShader);

	// render the GUI
	gui->render();

	// Get any errors from OpenGL
	auto error = glGetError();
	if ( error != GL_NO_ERROR ) {
		std::string val = errorString( error );
		std::cout<< "Error initializing OpenGL! " << error << ", " << val << std::endl;
	}
}

std::string Graphics::errorString(GLenum error) {
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
