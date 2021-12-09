#ifndef PHYSICS_H
#define PHYSICS_H

#include "arguments.h"
#include "graphics_headers.h"

#include <map>
#include <functional>
#include <btBulletDynamicsCommon.h>


// Uncomment to enable physics debug rendering
// #define PHYSICS_DEBUG


// Function which converts an origin and directection, to a start and end point
inline std::pair<glm::vec3, glm::vec3> dir2end(glm::vec3 origin, glm::vec3 direction, float length = 1024) { return { origin, origin + glm::normalize(direction) * length }; }


// Forward declarations
class Engine;
class Object;
class Shader;
class Camera;
#ifdef PHYSICS_DEBUG
class BulletDebugDrawer_OpenGL;
#endif // PHYSICS_DEBUG

// Class which provides the physics engine
class Physics {//: public btEventListener {
public:
	// using ContactEvent = std::function<void(const btCollisionCallback::ContactPair&)>;
	using ContactEvent = std::function<void()>;

	// Gets the singleton
	static Physics& getSingleton();

public:
	Physics(std::shared_ptr<Object>& sceneRoot);
	~Physics();
	bool initialize(Engine* engine, const Arguments& args);
	void update(float dt);
#ifdef PHYSICS_DEBUG
	void render(Camera* camera);
#endif

	// virtual void onContact(const btCollisionCallback::CallbackData& callbackData) override;

	// Get the world
	btDiscreteDynamicsWorld& getWorld() { return world; }

	void addContactCallback(std::shared_ptr<Object>& obj, ContactEvent e);
	void addContactCallback(std::shared_ptr<Object>&& obj, ContactEvent e) { addContactCallback(obj, e); }

protected:
	// Singleton
	static Physics* singleton;

	// Scene root
	std::shared_ptr<Object>& sceneRoot;

protected:
	// Bullet Variables
	btDefaultCollisionConfiguration config;
	btCollisionDispatcher dispatcher;
	btDbvtBroadphase broadphase;
	btSequentialImpulseConstraintSolver solver;
	btDiscreteDynamicsWorld world;

	// std::map<uint32_t, ContactEvent> contactEvents;

#ifdef PHYSICS_DEBUG
	// Debug rendering variables
	Shader* lineShader;
	std::unique_ptr<BulletDebugDrawer_OpenGL> debugDrawer;
#endif
};

#ifdef PHYSICS_DEBUG

// Helper class; draws the world as seen by Bullet.
// This is very handy to see it Bullet's world matches yours
// How to use this class :
// Declare an instance of the class :
// 
// dynamicsWorld->setDebugDrawer(&mydebugdrawer);
// Each frame, call it :
//
// mydebugdrawer.setMatrices(ViewMatrix, ProjectionMatrix);
// dynamicsWorld->debugDrawWorld();
class BulletDebugDrawer_OpenGL : public btIDebugDraw {
	static GLuint VBO, VAO;

	std::vector<std::pair<glm::vec3, glm::vec3>> lines;
	
public:
	BulletDebugDrawer_OpenGL();

	void render(Shader* lineShader, glm::mat4 pViewMatrix, glm::mat4 pProjectionMatrix);
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}
	virtual void reportErrorWarning(const char* warning) { std::cerr << warning << std::endl; }
	virtual void draw3dText(const btVector3 &, const char *) {}
	virtual void setDebugMode(int p) { m = p; }
	int getDebugMode(void) const { return m; }
	int m;
};
#endif


// -- Bullet Interop --


// Convert vector3s
static btVector3 toBullet(const glm::vec3& v) { return { v.x, v.y, v.z }; }
static btVector3 toBullet(const glm::vec3&& v) { return toBullet(v); }
// Convert quaternions
static btQuaternion toBullet(const glm::quat& q) { return { q.x, q.y, q.z, q.w }; }
static btQuaternion toBullet(const glm::quat&& q) { return toBullet(q); }
// Convert vec3 and quaternion to transform
static btTransform toBullet(const glm::vec3 pos, const glm::quat rotation){ return btTransform(toBullet(rotation), toBullet(pos)); }
// Convert mat4 to transform
static btTransform toBullet(const glm::mat4& mat){
	btTransform out;
	out.setFromOpenGLMatrix(glm::value_ptr(mat));
	return btTransform(out);
}
static btTransform toBullet(const glm::mat4&& mat){ return toBullet(mat); }

// Convert vector3s
static glm::vec3 toGLM(const btVector3& v) { return {v.getX(), v.getY(), v.getZ()}; }
static glm::vec3 toGLM(const btVector3&& v) { return toGLM(v); }
// Convert quaternions
static glm::quat toGLM(const btQuaternion& q){ return { q.getX(), q.getY(), q.getZ(), q.getW() }; }
static glm::quat toGLM(const btQuaternion&& q){ return toGLM(q); }
// Convert transform to mat4
static glm::mat4 toGLM(const btTransform& t){
	glm::mat4 out;
	t.getOpenGLMatrix(glm::value_ptr(out));
	return out;
}
static glm::mat4 toGLM(const btTransform&& t){ return toGLM(t); }

#endif /* PHYSICS_H */
