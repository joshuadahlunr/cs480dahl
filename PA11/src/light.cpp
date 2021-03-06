#include "light.h"
#include "shader.h"

// Set the static count so shaders on the GPU know how many to process
size_t Light::count = 0;

void Light::update(float dt) {
	setModel(glm::mat4(1));
	Object::setPosition(getParent()->getPosition() + position);
}

void Light::render(Shader* boundShader) {
	glm::vec4 lightPosition = glm::vec4(getPosition(), 1);

	// Bind all of the variables in the shader
	glUniform1ui(boundShader->getUniformLocation(uniformLocationLightType.c_str()), (int) type);
	if(!uniformLocationAmbient.empty()) glUniform4fv(boundShader->getUniformLocation(uniformLocationAmbient.c_str()), 1, glm::value_ptr(lightAmbient));
	if(!uniformLocationDiffuse.empty()) glUniform4fv(boundShader->getUniformLocation(uniformLocationDiffuse.c_str()), 1, glm::value_ptr(lightDiffuse));
	if(!uniformLocationSpecular.empty()) glUniform4fv(boundShader->getUniformLocation(uniformLocationSpecular.c_str()), 1, glm::value_ptr(lightSpecular));
	if(!uniformLocationPosition.empty()) glUniform4fv(boundShader->getUniformLocation(uniformLocationPosition.c_str()), 1, glm::value_ptr(lightPosition));
	if(!uniformLocationDirection.empty()) glUniform3fv(boundShader->getUniformLocation(uniformLocationDirection.c_str()), 1, glm::value_ptr(lightDirection));
	if(!uniformLocationLightCutoffAngleCosine.empty()) glUniform1f(boundShader->getUniformLocation(uniformLocationLightCutoffAngleCosine.c_str()), lightCutoffAngleCosine);
	if(!uniformLocationLightIntensity.empty()) glUniform1f(boundShader->getUniformLocation(uniformLocationLightIntensity.c_str()), lightIntensity);
	if(!uniformLocationLightFalloff.empty()) glUniform1f(boundShader->getUniformLocation(uniformLocationLightFalloff.c_str()), lightFalloff);
	if(!uniformLocationLightAttenuationStartDistance.empty()) glUniform1f(boundShader->getUniformLocation(uniformLocationLightAttenuationStartDistance.c_str()), lightAttenuationStartDistance);

	// render base class
	Object::render(boundShader);
}

AmbientLight::AmbientLight(std::string lightVariable /* = "lights"*/) : Light(Light::Type::Ambient, setupID()) {
	uniformLocationLightType = lightVariable + "[" + std::to_string(id) + "].type";
	uniformLocationAmbient = lightVariable + "[" + std::to_string(id) + "].ambient";
}

// Memory backing the primary directional light
DirectionalLight* DirectionalLight::primary = nullptr;

DirectionalLight::DirectionalLight(std::string lightVariable /* = "lights"*/) : Light(Light::Type::Directional, setupID()) {
	uniformLocationLightType = lightVariable + "[" + std::to_string(id) + "].type";
	uniformLocationAmbient = lightVariable + "[" + std::to_string(id) + "].ambient";
	uniformLocationDiffuse = lightVariable + "[" + std::to_string(id) + "].diffuse";
	uniformLocationSpecular = lightVariable + "[" + std::to_string(id) + "].specular";
	uniformLocationDirection = lightVariable + "[" + std::to_string(id) + "].direction";
	uniformLocationLightAttenuationStartDistance = lightVariable + "[" + std::to_string(id) + "].attenuationDistance";

	// Default attenuation for directional lights is infinty
	lightAttenuationStartDistance = INFINITY;

	// If we are the first directional light mark us as primary
	if(!primary) primary = this;
}

PointLight::PointLight(std::string lightVariable /* = "lights"*/) : Light(Light::Type::Point, setupID()) {
	uniformLocationLightType = lightVariable + "[" + std::to_string(id) + "].type";
	uniformLocationAmbient = lightVariable + "[" + std::to_string(id) + "].ambient";
	uniformLocationDiffuse = lightVariable + "[" + std::to_string(id) + "].diffuse";
	uniformLocationSpecular = lightVariable + "[" + std::to_string(id) + "].specular";
	uniformLocationPosition = lightVariable + "[" + std::to_string(id) + "].position";
	uniformLocationLightAttenuationStartDistance = lightVariable + "[" + std::to_string(id) + "].attenuationDistance";
}

SpotLight::SpotLight(std::string lightVariable /* = "lights"*/) : Light(Light::Type::Spot, setupID()) {
	uniformLocationLightType = lightVariable + "[" + std::to_string(id) + "].type";
	uniformLocationAmbient = lightVariable + "[" + std::to_string(id) + "].ambient";
	uniformLocationDiffuse = lightVariable + "[" + std::to_string(id) + "].diffuse";
	uniformLocationSpecular = lightVariable + "[" + std::to_string(id) + "].specular";
	uniformLocationPosition = lightVariable + "[" + std::to_string(id) + "].position";
	uniformLocationDirection = lightVariable + "[" + std::to_string(id) + "].direction";
	uniformLocationLightCutoffAngleCosine = lightVariable + "[" + std::to_string(id) + "].cutoffAngleCosine";
	uniformLocationLightIntensity = lightVariable + "[" + std::to_string(id) + "].intensity";
	uniformLocationLightFalloff = lightVariable + "[" + std::to_string(id) + "].falloff";
	uniformLocationLightAttenuationStartDistance = lightVariable + "[" + std::to_string(id) + "].attenuationDistance";
}
