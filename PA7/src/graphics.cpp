#include "graphics.h"
#include "engine.h"
#include "camera.h"

#include "celestial.h"
#include "rings.h"
#include "skybox.h"
#include <fstream>

Graphics::Graphics() { }

Graphics::~Graphics() {
	// Ensure that the scene root isn't leaked
	delete sceneRoot;
	sceneRoot = nullptr;
}

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
	m_camera = new Camera();
	if(!m_camera->Initialize(width, height)) {
		printf("Camera Failed to Initialize\n");
		return false;
	}

	// Give camera access to graphics
	m_camera->setGraphics(this);

	// Hookup camera mouse and keyboard events
	engine->keyboardEvent += [&](auto event) { m_camera->Keyboard(event); };
	engine->mouseButtonEvent += [&](auto event) { m_camera->MouseButton(event); };
	engine->mouseMotionEvent += [&](auto event) { m_camera->MouseMotion(event); };
	engine->mouseWheelEvent += [&](auto event) { m_camera->MouseWheel(event); };

	// Create the celestials
	sceneRoot = CelestialFromJson(args, args.getConfig()["Scene"]);

	// Focus the camera on a celestial
	m_camera->setFocusCelestial(celestials.back());

	// Set up the shaders
	m_shader = new Shader();
	if(!m_shader->Initialize()) {
		printf("Shader Failed to Initialize\n");
		return false;
	}

	// Add the vertex shader
	if(!m_shader->AddShader(GL_VERTEX_SHADER, args.getVertexFilePath(), args)) {
		printf("Vertex Shader failed to Initialize\n");
		return false;
	}

	// Add the fragment shader
	if(!m_shader->AddShader(GL_FRAGMENT_SHADER, args.getFragmentFilePath(), args)) {
		printf("Fragment Shader failed to Initialize\n");
		return false;
	}

	// Connect the program
	if(!m_shader->Finalize()) {
		printf("Program failed to Finalize\n");
		return false;
	}

	// Locate the projection matrix in the shader
	m_projectionMatrix = m_shader->GetUniformLocation("projectionMatrix");
	if (m_projectionMatrix == INVALID_UNIFORM_LOCATION) {
		printf("m_projectionMatrix not found\n");
		return false;
	}

	// Locate the view matrix in the shader
	m_viewMatrix = m_shader->GetUniformLocation("viewMatrix");
	if (m_viewMatrix == INVALID_UNIFORM_LOCATION) {
		printf("m_viewMatrix not found\n");
		return false;
	}

	// Locate the model matrix in the shader
	m_modelMatrix = m_shader->GetUniformLocation("modelMatrix");
	if (m_modelMatrix == INVALID_UNIFORM_LOCATION) {
		printf("m_modelMatrix not found\n");
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

// Helper function which converts a json array into a glm::vec3
// Provides an optional default value in case the provided json is null
glm::vec3 jsonToVec3(json j, glm::vec3 _default = glm::vec3(0)){
	if(j.is_null()) return _default;

	glm::vec3 out;
	out.x = j[0];
	out.y = j[1];
	out.z = j[2];
	return out;
}

// Helper function which converts a json array into a glm::vec3
// Provides an optional default value in case the provided json is null
glm::vec2 jsonToVec2(json j, glm::vec2 _default = glm::vec2(0)){
	if(j.is_null()) return _default;

	glm::vec2 out;
	out.x = j[0];
	out.y = j[1];
	return out;
}

// Helper function which converts a json array into a float
// Provides an optional default value in case the provided json is null
float jsonToFloat(json j, float _default = 0) {
	if(j.is_null()) return _default;

	return j;
}

// Recursively initializes a scene tree from the provided json data
Celestial* Graphics::CelestialFromJson(const Arguments& args, json j, uint depth) {
	// Create a new celestial object
	Celestial* celestial = new Celestial();
	celestial->sceneDepth = depth;
	celestial->celestialRadius = j.value("Mean Radius (km)", 1);

	// Distance can be provided as a single number or a pair
	auto od = j["Mean Orbit Radius (km)"];
	if(od.is_number()) celestial->orbitDistance = glm::vec2((float) od);
	else if(od.is_array()) celestial->orbitDistance = jsonToVec2(od);
	else celestial->orbitDistance = glm::vec2(0);

	// Set the properties of the newly created object
	celestial->orbitSpeed = jsonToFloat(j["Orbit Period (d)"]);
	celestial->orbitSpeed = 360.0 / (((celestial->orbitSpeed)/365.0) * 60.0);
	if(isinf(celestial->orbitSpeed)) celestial->orbitSpeed = 0;

	//celestial->orbitSpeed = celestial->orbitSpeed/(60 * 60 * 360);
	celestial->orbitInitialOffset = jsonToFloat(j["Orbit Initial Offset"]);
	celestial->eclipticInclination = jsonToFloat(j["Ecliptic Inclination"]);
	celestial->orbitalTiltNormal = jsonToVec3(j["Orbital Tilt Normal"], glm::vec3(0, 1, 0));
	celestial->rotationSpeed = jsonToFloat(j["Sidereal Rotation Period (d)"]);
	celestial->rotationSpeed = 360.0 / (((celestial->rotationSpeed)/365.0) * 60.0);
	if(isinf(celestial->rotationSpeed)) celestial->rotationSpeed = 0;
	celestial->axialTiltNormal = jsonToVec3(j["Axial Tilt Normal"], glm::vec3(0, 1, 0));
	std::cout << celestial->rotationSpeed << std::endl;

	// Initialize the celestial and set its texture
	std::string texturePath = j.value("Texture Path", "textures/invalid.png");
	celestial->Initialize(args, args.getResourcePath() + texturePath);

	// If there is a ring defined
	if(j.contains("Ring")){
		// Create a child ring and set its properties
		Ring* ring = (Ring*) celestial->addChild(new Ring());
		ring->innerRadius = jsonToFloat(j["Ring"]["Inner Radius (km)"], 1);
		ring->outerRadius = jsonToFloat(j["Ring"]["Outer Radius (km)"], 2);
		ring->resolution = j["Ring"].value("Resolution", 128);
		ring->tilt = jsonToVec3(j["Ring"]["Tilt"], glm::vec3(0, 1, 0));

		// Initialize the ring and set its texture
		std::string texturePath = j["Ring"].value("Texture Path", "textures/invalid.png");
		ring->Initialize(args, args.getResourcePath() + texturePath);
	}

	// Recursively initialize the celestial's children
	for (auto child: j["Children"])
		celestial->addChild(CelestialFromJson(args, child, depth + 1));

	// Add all celestials to a list for round robin indexing
	celestials.push_back(celestial);

	return celestial;
}

void Graphics::Update(unsigned int dt) {
	// Update the object
	sceneRoot->Update(dt);

	// Update the camera
	m_camera->Update(dt);
}

void Graphics::Render() {
	//clear the screen
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render the skybox first (thus everything else is drawn in front of it)
	m_skybox->Render();

	// Start the correct program
	m_shader->Enable();

	// Send in the projection and view to the shader
	glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
	glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));

	// Render the object
	sceneRoot->Render(m_modelMatrix);

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
