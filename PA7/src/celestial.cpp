#include "celestial.h"

bool Celestial::Initialize(const Arguments& args, const std::string& texturePath) {
	bool success = true;

	// Get the showcase path from the arguments
	std::string filepath = args.getShowcaseModelPath();
	// If the filepath doesn't already have the shader directory path, add the shader dirrectory path
	std::string modelDirectory = args.getResourcePath() + "models/";
	if(filepath.find(modelDirectory) == std::string::npos)
		filepath = modelDirectory + filepath;

	// Load the model
	success &= LoadModelFile(args, filepath);

	// Load the texture
	glDeleteTextures(1, &tex);
	success &= LoadTextureFile(args, texturePath, false);

	// Apply our initial offset
	orbitAngle = orbitInitialOffset;

	// Create and initialize the orbit path
	orbitPath = new OrbitPath();
	orbitPath->Initialize(args, texturePath, orbitDistance, orbitalTiltNormal);

	return success;
}

void Celestial::Update(unsigned int dt) {
	// Update our angle with respect to rotation and orbit
	orbitAngle += dt * milliToSec * orbitSpeed * globalTimeScale;
	rotationAngle += dt * milliToSec * rotationSpeed * globalTimeScale;

	// Find our current position along an elipse
	glm::vec3 translation(glm::cos(glm::radians(orbitAngle)) * orbitDistance.x, 0, glm::sin(glm::radians(orbitAngle)) * orbitDistance.y);

	// Appropriately scale, rotate, tilt, and translate ourselves
	glm::mat4 planarModel = glm::translate(glm::mat4(1.0f), translation);
	planarModel = planarModel * rotateTo(glm::vec3(0, 1, 0), axialTiltNormal);
	planarModel = glm::rotate(planarModel, glm::radians(rotationAngle), glm::vec3(0, 1, 0));
	planarModel = glm::scale(planarModel, glm::vec3(celestialRadius));

	// The operations so far have occurred in a plane, tilt the plane so that not all planets are in the same plane
	glm::mat4 orbitalTiltModel = rotateTo(glm::vec3(0, 1, 0), orbitalTiltNormal);

	// Apply a model with every operation as our model matrix (based on the parent's model matrix)
	setModelRelativeToParent( orbitalTiltModel * planarModel );
	// Specify a simpler model matrix without rotations as the base for children
	setChildModelRelativeToParent( orbitalTiltModel * glm::translate(glm::mat4(1.0f), translation) );

	Object::Update(dt);
}

void Celestial::Render(GLint modelMatrix) {
	// Render the orbit path. Not a child, so needs to be called here
	orbitPath->Render(modelMatrix);

	Object::Render(modelMatrix);
}
