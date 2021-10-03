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

	glDeleteTextures(1, &tex);
	success &= LoadTextureFile(args, texturePath);

	orbitAngle = orbitInitialOffset;

	return success;
}

// Function which returns the matrix needed to rotate the <original> vector into the <target> vector 
glm::mat4 rotateTo(glm::vec3 original, glm::vec3 target){
	glm::quat q;
	glm::vec3 a = glm::cross(original, target);
	q.x = a.x;
	q.y = a.y;
	q.z = a.z;
	
	float originalLength = glm::length(original);
	float targetLength = glm::length(target);
	q.w = glm::sqrt((originalLength * originalLength) * (targetLength * targetLength) + glm::dot(original, target));

	q = glm::normalize(q);
	return glm::mat4_cast(q);
}

void Celestial::Update(unsigned int dt) {
	// std::cout << m_reverseOrbit << " - " << m_reverseRotation << " - " << pauseOrbit << " - " << pauseRotation << std::endl;
	orbitAngle += dt * milliToSec * orbitSpeed;
	rotationAngle += dt * milliToSec * rotationSpeed;

	glm::vec3 translation(glm::cos(glm::radians(orbitAngle)) * orbitDistance, 0, glm::sin(glm::radians(orbitAngle)) * orbitDistance);

	glm::mat4 planarModel = glm::translate(glm::mat4(1.0f), translation);
	planarModel = planarModel * rotateTo(glm::vec3(0, 1, 0), axialTiltNormal);
	planarModel = glm::rotate(planarModel, rotationAngle, glm::vec3(0, 1, 0));
	planarModel = glm::scale(planarModel, glm::vec3(celestialRadius));

	glm::mat4 orbitalTiltModel = rotateTo(glm::vec3(0, 1, 0), orbitalTiltNormal);
	setModelRelativeToParent( orbitalTiltModel * planarModel );
	setChildModelRelativeToParent( orbitalTiltModel * glm::translate(glm::mat4(1.0f), translation) );
	
	Object::Update(dt);
}