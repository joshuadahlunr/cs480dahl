#ifndef LIGHT_LOCAL_H
#define LIGHT_LOCAL_H

#include "object.h"

class Light : public Object {
public:
	enum Type : unsigned int {
		Disabled = 0,
		Ambient = 1,
		Directional = 2,
		Point = 3,
		Spot = 4
	};
public:
	static size_t count;

	Light() : Light(Type::Disabled) {}
	Light(Type type, size_t id = 0) : type(type), id(id) {}
	void Render(Shader* boundShader) override;
	void Update(unsigned int dt) override;

	void setAmbient(glm::vec4 color) {lightAmbient = color;}
	void setDiffuse(glm::vec4 color) {lightDiffuse = color;}
	void setSpecular(glm::vec4 color) {lightSpecular = color;}
	void setDirection(glm::vec3 direction) {lightDirection = direction;}
	void setCutoffAngle(float angle) { setCutoffAngleCosine(glm::cos(angle)); }
	void setCutoffAngleCosine(float cos) { lightCutoffAngleCosine = cos; }
	void setIntensity(float intensity) { lightIntensity = intensity; }
	void setFalloff(float falloff) { lightFalloff = falloff; }
	void setPosition(glm::vec3 pos) { position = pos; }
protected:
	int setupID() { std::cout << count << std::endl; return count++; }

protected:
	const size_t id;

	std::string uniformLocationLightType;
	std::string uniformLocationAmbient;
	std::string uniformLocationDiffuse;
	std::string uniformLocationSpecular;
	std::string uniformLocationPosition;
	std::string uniformLocationDirection;
	std::string uniformLocationLightCutoffAngleCosine;
	std::string uniformLocationLightIntensity;
	std::string uniformLocationLightFalloff;
	// const std::string uniformLocationNumLights;

public:
	const Type type = Type::Ambient;
	glm::vec4 lightAmbient = glm::vec4(0, 0, 0, 1.0f);
	glm::vec4 lightDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 lightSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightDirection = glm::vec3(0, -1, 0);
	float lightCutoffAngleCosine = 0.9659258263;
    float lightIntensity = 1;
    float lightFalloff = .1;

	glm::vec3 position;
};

class AmbientLight: public Light {
public:
	AmbientLight(std::string lightVariable = "lights");
};

// Global light object for a single light source across the whole scene
class DirectionalLight : public Light {
public:
	DirectionalLight(std::string lightVariable = "lights");
};

// Point light object for a light source casting in all directions
class PointLight : public Light {
public:
	PointLight(std::string lightVariable = "lights");

protected:
	static int count;
};

// Spotlight object for a light source constrained to some 3d cone volume
class SpotLight : public Light {
public:
	SpotLight(std::string lightVariable = "lights");

protected:
	static int count;
};

#endif /* LIGHT_H */
