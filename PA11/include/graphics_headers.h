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
#include <glm/gtx/norm.hpp>

#define INVALID_UNIFORM_LOCATION 0x7fffffff

// Struct defining the vertecies sent to OpenGL
struct Vertex {
	glm::vec3 vertex;
	glm::vec3 color;
	glm::vec2 uv;
	glm::vec3 normal;

	Vertex(glm::vec3 p, glm::vec3 c, glm::vec2 u, glm::vec3 n): vertex(p), color(c), uv(u), normal(n) {}
};

// Struct defining collision mesh data
struct CollisionMesh {
	float* vertexData = nullptr;
	int* indiceData = nullptr;
	int numVertices;

	void clear() {
		if(vertexData) delete [] vertexData;
		vertexData = nullptr;

		if(indiceData) delete [] indiceData;
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

	ConvexCollisionMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indicies) { }

	~ConvexCollisionMesh() {

		if(convexVertexData) delete convexVertexData;
		convexVertexData = nullptr;
		if(convexIndiceData) delete convexIndiceData;
		convexIndiceData = nullptr;
	}
};

// Function which returns the matrix needed to rotate the <original> std::vector into the <target> std::vector
static glm::quat rotateTo(glm::vec3 original, glm::vec3 target){
	glm::quat q;
	glm::vec3 a = glm::cross(original, target);
	q.x = a.x;
	q.y = a.y;
	q.z = a.z;

	float originalLength = glm::length(original);
	float targetLength = glm::length(target);
	q.w = glm::sqrt((originalLength * originalLength) * (targetLength * targetLength) + glm::dot(original, target));

	q = glm::normalize(q);
	return q;
}

// Function which constructs an identity quaternion
namespace glm { static glm::quat quat_identity(){ return glm::quat(1, 0, 0, 0); } }

// Function which constructs a model matrix from a translation, rotation, and scale
static glm::mat4 constructMat4(glm::vec3 translation, glm::quat rotation = glm::quat_identity(), glm::vec3 scale = glm::vec3(1)) {
	return glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale);
}

#endif /* GRAPHICS_HEADERS_H */
