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

#include <reactphysics3d/collision/PolygonVertexArray.h>

// Struct defining the vertecies sent to OpenGL
struct Vertex {
	glm::vec3 vertex;
	glm::vec3 color;
	glm::vec2 uv;
	glm::vec3 normal;

	Vertex(glm::vec3 p, glm::vec3 c, glm::vec2 u, glm::vec3 n): vertex(p), color(c), uv(u), normal(n) {}

	bool operator==(const Vertex& other) const { return vertex == other.vertex && color == other.color && uv == other.uv && normal == other.normal; }
};

// Function to hash a vertex
namespace std {
	template <>
	struct hash<Vertex> {
		size_t operator()(const Vertex& v) const {
			std::hash<float> hash;
			return hash(
				hash(v.vertex.x) * 7 + hash(v.vertex.y) * 13 + hash(v.vertex.z) * 19
				+ hash(v.color.r) * 29 + hash(v.color.g) * 37 + hash(v.color.b) * 43
				+ hash(v.uv.x) * 53 + hash(v.uv.y) * 61
				+ hash(v.normal.x) * 71 + hash(v.normal.y) * 79 + hash(v.normal.z) * 89
			);
		}
	};
}

struct CollisionMesh {	
	enum Type {
		Convex,
		Concave
	} type;

	CollisionMesh(Type type): type(type) {}
};

// Struct defining collision mesh data
struct ConcaveCollisionMesh : public CollisionMesh {
	float* vertexData = nullptr;
	int numVertices;
	int* indiceData = nullptr;
	int numIndices;

	void clear() {
		if(vertexData) delete [] vertexData;
		vertexData = nullptr;

		if(indiceData) delete [] indiceData;
		indiceData = nullptr;
	}

	ConcaveCollisionMesh() : CollisionMesh(Concave) {}

	~ConcaveCollisionMesh() {
		clear();
	}
};

struct ConvexCollisionMesh : public CollisionMesh {
	std::vector<float> points;
	std::vector<int> indices;
	std::vector<reactphysics3d::PolygonVertexArray::PolygonFace> faces;
	
	ConvexCollisionMesh() : CollisionMesh(Convex) {}
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
