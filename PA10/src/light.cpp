#include "light.h"
#include "shader.h"

// Set the static count so shaders on the GPU know how many to process
int PointLight::count = 0;
int SpotLight::count = 0;

GlobalLight::GlobalLight() : 
	// Specify constant uniform location variable names
	uniformLocationAmbient("globallight.ambient"),
	uniformLocationDiffuse("globallight.diffuse"),
	uniformLocationSpecular("globallight.specular"),
	uniformLocationPosition("globallight.position"),
	uniformLocationDirection("globallight.direction")
{
	std::cout << uniformLocationDiffuse << std::endl;
}

void GlobalLight::Render(Shader* boundShader) {
	// Specify constant uniform location variable names
	glm::vec4 lightPosition = glm::vec4(getPosition(), 1) + glm::vec4(0, 5, 0, 0);
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationAmbient.c_str()), 1, glm::value_ptr(lightAmbient));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationDiffuse.c_str()), 1, glm::value_ptr(lightDiffuse));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationSpecular.c_str()), 1, glm::value_ptr(lightSpecular));
	glUniform3fv(boundShader->GetUniformLocation(uniformLocationPosition.c_str()), 1, glm::value_ptr(lightPosition));

	// Render base class
	Object::Render(boundShader);
}

PointLight::PointLight() : 
	// Specify constant uniform location variable names
	id(PointLight::count++),
	lightType("pointlights["),
	uniformLocationAmbient(lightType + std::to_string(id) + "].ambient"),
	uniformLocationDiffuse(lightType + std::to_string(id) + "].diffuse"),
	uniformLocationSpecular(lightType + std::to_string(id) + "].specular"),
	uniformLocationPosition(lightType + std::to_string(id) + "].position"),
	uniformLocationNumLights("num_pointlights")
{
	std::cout << uniformLocationDiffuse << std::endl;
}

void PointLight::Render(Shader* boundShader) {
	// Specify constant uniform location variable names
	glm::vec4 lightPosition = glm::vec4(getPosition(), 1) + glm::vec4(0, 5, 0, 0);
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationAmbient.c_str()), 1, glm::value_ptr(lightAmbient));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationDiffuse.c_str()), 1, glm::value_ptr(lightDiffuse));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationSpecular.c_str()), 1, glm::value_ptr(lightSpecular));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationPosition.c_str()), 1, glm::value_ptr(lightPosition));

	glUniform1i(boundShader->GetUniformLocation(uniformLocationNumLights.c_str()), count);

	// Render base class
	Object::Render(boundShader);
}

SpotLight::SpotLight() : 
	id(SpotLight::count++),
	lightType("spotlights["),
	uniformLocationAmbient(lightType + std::to_string(id) + "].ambient"),
	uniformLocationDiffuse(lightType + std::to_string(id) + "].diffuse"),
	uniformLocationSpecular(lightType + std::to_string(id) + "].specular"),
	uniformLocationPosition(lightType + std::to_string(id) + "].position"),
	uniformLocationDirection(lightType + std::to_string(id) + "].direction"),
	uniformLocationCutoffAngleCosine(lightType + std::to_string(id) + "].cutoffAngleCosine"),
	uniformLocationIntensity(lightType + std::to_string(id) + "].intensity"),
	uniformLocationFalloff(lightType + std::to_string(id) + "].falloff"),
	uniformLocationNumLights("num_spotlights")
{
	std::cout << uniformLocationDiffuse << std::endl;
}

void SpotLight::Render(Shader* boundShader) {
	glm::vec4 lightPosition = glm::vec4(getPosition(), 1) + glm::vec4(0, 5, 0, 0);
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationAmbient.c_str()), 1, glm::value_ptr(lightAmbient));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationDiffuse.c_str()), 1, glm::value_ptr(lightDiffuse));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationSpecular.c_str()), 1, glm::value_ptr(lightSpecular));
	glUniform4fv(boundShader->GetUniformLocation(uniformLocationPosition.c_str()), 1, glm::value_ptr(lightPosition));
	glUniform3fv(boundShader->GetUniformLocation(uniformLocationDirection.c_str()), 1, glm::value_ptr(lightDirection));
	glUniform1f(boundShader->GetUniformLocation(uniformLocationCutoffAngleCosine.c_str()), lightCutoffAngleCosine);
	glUniform1f(boundShader->GetUniformLocation(uniformLocationIntensity.c_str()), lightIntensity);
	glUniform1f(boundShader->GetUniformLocation(uniformLocationFalloff.c_str()), lightFalloff);

	glUniform1i(boundShader->GetUniformLocation(uniformLocationNumLights.c_str()), count);

	// Render base class
	Object::Render(boundShader);
}