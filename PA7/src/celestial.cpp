#include "celestial.h"

#include "orbitPath.h"
#include <cmath>

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

	// Create and initialize the orbit path (if this isn't the sun)
	if(!sceneDepth == 0) {
		OrbitPath* orbitPath = (OrbitPath*) addChild(new OrbitPath());
		success &= orbitPath->Initialize(args, texturePath, scaledOrbitDistance(), orbitDistance, orbitalTiltNormal, sceneDepth);
	}

	// Apply our initial offset
	orbitAngle = rand() % 360; //orbitInitialOffset;

	return success;
}

void Celestial::Update(unsigned int dt) {
	// Update our angle with respect to rotation and orbit
	orbitAngle += dt * milliToSec * orbitSpeed * globalTimeScale;
	rotationAngle += dt * milliToSec * rotationSpeed * globalTimeScale;

	// Find our current position along an elipse
	glm::vec2 distance = scaledOrbitDistance();
	glm::vec3 translation(glm::cos(glm::radians(orbitAngle)) * distance.x, 0, glm::sin(glm::radians(orbitAngle)) * distance.y);

	// Appropriately scale, rotate, tilt, and translate ourselves
	glm::mat4 planarModel = glm::translate(glm::mat4(1.0f), translation);
	planarModel = planarModel * rotateTo(glm::vec3(0, 1, 0), axialTiltNormal);
	planarModel = glm::rotate(planarModel, glm::radians(rotationAngle), glm::vec3(0, 1, 0));
	planarModel = glm::scale(planarModel, glm::vec3( scaledRadius() ));

	// The operations so far have occurred in a plane, tilt the plane so that not all planets are in the same plane
	glm::mat4 orbitalTiltModel = rotateTo(glm::vec3(0, 1, 0), orbitalTiltNormal);

	// Apply a model with every operation as our model matrix (based on the parent's model matrix)
	setModelRelativeToParent( orbitalTiltModel * planarModel );
	// Specify a simpler model matrix without rotations as the base for children
	setChildModelRelativeToParent( orbitalTiltModel * glm::translate(glm::mat4(1.0f), translation) );

	Object::Update(dt);
}


float Celestial::scaledRadius(){
	if(!globalShouldScale) return celestialRadius / 100000;

	return log(celestialRadius) / log(2) - 10;
}

glm::vec2 Celestial::scaledOrbitDistance() {
	if(!globalShouldScale) return orbitDistance / glm::vec2(100000.0);

	glm::vec2 out;
	out.x = (log(orbitDistance.x) / log(2) - 20) * 20;
	out.y = (log(orbitDistance.y) / log(2) - 20) * 20;
	if(isinf(out.x)) out.x = 0;
	if(isinf(out.y)) out.y = 0;
	return out;
}