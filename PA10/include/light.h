#ifndef LIGHT_LOCAL_H
#define LIGHT_LOCAL_H

#include "object.h"

// Global light object for a single light source across the whole scene
class GlobalLight : public Object {
public:
	GlobalLight();
	
	void Render(Shader* boundShader);
	void setAmbient(glm::vec4 color) {lightAmbient = color;}
	void setDiffuse(glm::vec4 color) {lightDiffuse = color;}
	void setSpecular(glm::vec4 color) {lightSpecular = color;}
protected:
private:
	const std::string uniformLocationAmbient;
	const std::string uniformLocationDiffuse;
	const std::string uniformLocationSpecular;
	const std::string uniformLocationPosition;
	const std::string uniformLocationDirection;

	glm::vec4 lightAmbient = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
	glm::vec4 lightDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 lightSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPosition = glm::vec3(0, 0, 0);
	glm::vec3 lightDirection = glm::vec3(0, -1, 0);
};

// Point light object for a light source casting in all directions
class PointLight : public Object {
public:
	PointLight();
	void Render(Shader* boundShader);

	void setAmbient(glm::vec4 color) {lightAmbient = color;}
	void setDiffuse(glm::vec4 color) {lightDiffuse = color;}
	void setSpecular(glm::vec4 color) {lightSpecular = color;}
	void setDirection(glm::vec3 dir) {lightDirection = dir;}
	void setRadius(float rad) {radius = rad;}
protected:
private:
	static int count;
	const int id;

	const std::string lightType;
	const std::string uniformLocationAmbient;
	const std::string uniformLocationDiffuse;
	const std::string uniformLocationSpecular;
	const std::string uniformLocationPosition;
	const std::string uniformLocationRadius;
	const std::string uniformLocationNumLights;

	glm::vec4 lightAmbient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 lightDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 lightSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightDirection = glm::vec3(0, -1, 0);
	float radius = 5;
};

// Spotlight object for a light source constrained to some 3d cone volume
class SpotLight : public Object {
public:
	SpotLight();
	void Render(Shader* boundShader);
	void setAmbient(glm::vec4 color) {lightAmbient = color;}
	void setDiffuse(glm::vec4 color) {lightDiffuse = color;}
	void setSpecular(glm::vec4 color) {lightSpecular = color;}
	void setDirection(glm::vec3 dir) {lightDirection = dir;}
protected:
private:
	static int count;
	const int id;

	const std::string lightType;
	const std::string uniformLocationAmbient;
	const std::string uniformLocationDiffuse;
	const std::string uniformLocationSpecular;
	const std::string uniformLocationPosition;
	const std::string uniformLocationDirection;
	const std::string uniformLocationCutoffAngleCosine;
	const std::string uniformLocationIntensity;
	const std::string uniformLocationFalloff;
	const std::string uniformLocationNumLights;

	glm::vec4 lightAmbient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 lightDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 lightSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightDirection = glm::vec3(0, -1, 0);
	float lightCutoffAngleCosine = glm::cos(glm::radians(60.0));
	float lightIntensity = 5;
	float lightFalloff = 0.1f;
};

#endif /* LIGHT_H */
