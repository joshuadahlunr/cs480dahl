#include "light.h"
#include "shader.h"

// Set the static count so shaders on the GPU know how many to process
size_t Light::count = 0;

void Light::Update(float dt) {
	setModel(glm::mat4(1));
	Object::setPosition(getParent()->getPosition() + position);
}

void Light:: Render(Shader* boundShader){
	glm::vec4 lightPosition = glm::vec4(getPosition(), 1);

	// Bind all of the variables in the shader
	glUniform1ui(boundShader->GetUniformLocation(uniformLocationLightType.c_str()), (int) type);
	if(!uniformLocationAmbient.empty()) glUniform4fv(boundShader->GetUniformLocation(uniformLocationAmbient.c_str()), 1, glm::value_ptr(lightAmbient));
	if(!uniformLocationDiffuse.empty()) glUniform4fv(boundShader->GetUniformLocation(uniformLocationDiffuse.c_str()), 1, glm::value_ptr(lightDiffuse));
	if(!uniformLocationSpecular.empty()) glUniform4fv(boundShader->GetUniformLocation(uniformLocationSpecular.c_str()), 1, glm::value_ptr(lightSpecular));
	if(!uniformLocationPosition.empty()) glUniform4fv(boundShader->GetUniformLocation(uniformLocationPosition.c_str()), 1, glm::value_ptr(lightPosition));
	if(!uniformLocationDirection.empty()) glUniform3fv(boundShader->GetUniformLocation(uniformLocationDirection.c_str()), 1, glm::value_ptr(lightDirection));
	if(!uniformLocationLightCutoffAngleCosine.empty()) glUniform1f(boundShader->GetUniformLocation(uniformLocationLightCutoffAngleCosine.c_str()), lightCutoffAngleCosine);
	if(!uniformLocationLightIntensity.empty()) glUniform1f(boundShader->GetUniformLocation(uniformLocationLightIntensity.c_str()), lightIntensity);
	if(!uniformLocationLightFalloff.empty()) glUniform1f(boundShader->GetUniformLocation(uniformLocationLightFalloff.c_str()), lightFalloff);

	//std::cout << count << std::endl;

	// Render base class
	Object::Render(boundShader);
}

AmbientLight::AmbientLight(std::string lightVariable /* = "lights"*/) : Light(Light::Type::Ambient, setupID()) {
	uniformLocationLightType = lightVariable + "[" + std::to_string(id) + "].type";
	uniformLocationAmbient = lightVariable + "[" + std::to_string(id) + "].ambient";
}

DirectionalLight::DirectionalLight(std::string lightVariable /* = "lights"*/) : Light(Light::Type::Directional, setupID()) {
	uniformLocationLightType = lightVariable + "[" + std::to_string(id) + "].type";
	uniformLocationAmbient = lightVariable + "[" + std::to_string(id) + "].ambient";
	uniformLocationDiffuse = lightVariable + "[" + std::to_string(id) + "].diffuse";
	uniformLocationSpecular = lightVariable + "[" + std::to_string(id) + "].specular";
	uniformLocationDirection = lightVariable + "[" + std::to_string(id) + "].direction";
}

PointLight::PointLight(std::string lightVariable /* = "lights"*/) : Light(Light::Type::Point, setupID()) {
	uniformLocationLightType = lightVariable + "[" + std::to_string(id) + "].type";
	uniformLocationAmbient = lightVariable + "[" + std::to_string(id) + "].ambient";
	uniformLocationDiffuse = lightVariable + "[" + std::to_string(id) + "].diffuse";
	uniformLocationSpecular = lightVariable + "[" + std::to_string(id) + "].specular";
	uniformLocationPosition = lightVariable + "[" + std::to_string(id) + "].position";
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
}
