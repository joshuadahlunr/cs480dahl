#ifndef GRAPHICS_HEADERS_H
#define GRAPHICS_HEADERS_H

#include <iostream>
#include <vector>

// Include OpenGL
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#define GLM_ENABLE_EXPERIMENTAL
#if defined(__APPLE__) || defined(MACOSX)
#	include <OpenGL/gl3.h>
#	include <OpenGL/GLU.h>
#else //linux as default
#	include <GL/glew.h>
//# include <GL/glu.h>
#endif

// GLM for matricies
#define GLM_FORCE_SWIZZLE 
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#define INVALID_UNIFORM_LOCATION 0x7fffffff

// Struct defining the vertecies sent to OpenGL
struct Vertex {
	glm::vec3 vertex;
	glm::vec3 color;
	glm::vec2 uv;
	glm::vec3 normal;

	Vertex(glm::vec3 v, glm::vec3 c, glm::vec2 u, glm::vec3 n): vertex(v), color(c), uv(u), normal(n) {}
};

// Struct defining collision mesh data
struct CollisionMesh {
	float* vertexData = nullptr;
	int* indiceData = nullptr;
	int numVertices;

	void clear() {
		if(vertexData) delete vertexData;
		vertexData = nullptr;
		if(indiceData) delete indiceData;
		indiceData = nullptr;
	}

	~CollisionMesh() {
		clear();
	}
};

// Struct defining collision mesh data
struct ConvexCollisionMesh {
	float* convexVertexData = nullptr;
	int* convexIndiceData = nullptr;
	int numConvexVertices;

	ConvexCollisionMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indicies) {

	}

	~ConvexCollisionMesh() {

		if(convexVertexData) delete convexVertexData;
		convexVertexData = nullptr;
		if(convexIndiceData) delete convexIndiceData;
		convexIndiceData = nullptr;
	}
};

// Function which returns the matrix needed to rotate the <original> vector into the <target> vector
static glm::mat4 rotateTo(glm::vec3 original, glm::vec3 target){
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

// Global variable representing the current timescale
extern float globalTimeScale;
// Global variable tracking if we should be using scaled choordinates or not
extern bool globalShouldScale;

#endif /* GRAPHICS_HEADERS_H */
