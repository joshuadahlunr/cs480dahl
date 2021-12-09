#include "physics.h"
#include "engine.h"
#include "object.h"
#include "shader.h"
#include "camera.h"

// Backing/access for the singleton
Physics* Physics::singleton;
Physics& Physics::getSingleton() { return *singleton; }

Physics::Physics(Object::ptr& sceneRoot) : sceneRoot(sceneRoot), dispatcher(&config), world(&dispatcher, &broadphase, &solver, &config) { }
Physics::~Physics() {
#ifdef PHYSICS_DEBUG
	delete lineShader;
#endif
}

bool Physics::initialize(Engine* engine, const Arguments& args) {
	// Setup the singleton
	singleton = this;

	// Initalize Bullet
	// btDefaultCollisionConfiguration* config = new btDefaultCollisionConfiguration();
	// btCollisionDispatcher* dispatcher = new btCollisionDispatcher(config);
	// btBroadphaseInterface* broadphase = new btDbvtBroadphase();
	// btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	// btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config); 

	// Set gravity
	world.setGravity({0, -9.8, 0});

	// Debug Rendering
#ifdef PHYSICS_DEBUG
	lineShader = new Shader; // TODO: memory leak
	lineShader->initialize();
	lineShader->addShader(GL_VERTEX_SHADER, "physics.vert.glsl", args);
	lineShader->addShader(GL_FRAGMENT_SHADER, "physics.frag.glsl", args);
	lineShader->finalize();

	debugDrawer = std::make_unique<BulletDebugDrawer_OpenGL>();
	world.setDebugDrawer(debugDrawer.get());
	debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe); //| btIDebugDraw::DBG_DrawAabb);
#endif

	return true;
}

void Physics::update(float dt) {
	// Update the physics simulation
	world.stepSimulation(dt);
}

// void Physics::addContactCallback(Object::ptr& obj, ContactEvent e) { contactEvents[obj->getCollider().getEntity().id] = e; }

// void Physics::onContact(const rp3d::CollisionCallback::CallbackData& callbackData) {
// 	for(int i = 0; i < callbackData.getNbContactPairs(); i++)
// 		if(callbackData.getContactPair(i).getNbContactPoints() > 0)
// 			for(auto idEvent: contactEvents)
// 				if(callbackData.getContactPair(i).getCollider1()->getEntity().id == idEvent.first || callbackData.getContactPair(i).getCollider2()->getEntity().id == idEvent.first)
// 					idEvent.second(callbackData.getContactPair(i));
// }


#ifdef PHYSICS_DEBUG

// Render the world as seen by bullet
void Physics::render(Camera* camera) {
	// Bind the shader
	lineShader->enable();

	// Update matricies and draw
	world.debugDrawWorld();
	debugDrawer->render(lineShader, camera->getView(), camera->getProjection());
}


// -- Bullet Debug Drawing Implementation --


// Backing for the vertex buffer and attributes
GLuint BulletDebugDrawer_OpenGL::VBO;
GLuint BulletDebugDrawer_OpenGL::VAO;

BulletDebugDrawer_OpenGL::BulletDebugDrawer_OpenGL() {
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
}

void BulletDebugDrawer_OpenGL::render(Shader* lineShader, glm::mat4 pViewMatrix, glm::mat4 pProjectionMatrix)  {
	glUniformMatrix4fv(lineShader->getUniformLocation("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(pProjectionMatrix));
	glUniformMatrix4fv(lineShader->getUniformLocation("viewMatrix"), 1, GL_FALSE, glm::value_ptr(pViewMatrix));

	// Disable depth buffer
	glDepthMask(GL_FALSE);

	// Upload the lines
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3) * lines.size(), lines.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	// Draw the lines
	glDrawArrays(GL_LINES, 0, lines.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// Re-enable depth buffer
	glDepthMask(GL_TRUE);

	// Clear the lines for the next frame
	lines.clear();
}

void BulletDebugDrawer_OpenGL::drawLine(const btVector3& from, const btVector3& to, const btVector3& color){
	// Record the line
	lines.emplace_back(toGLM(from), toGLM(color));
	lines.emplace_back(toGLM(to), toGLM(color));
}

#endif // PHYSICS_DEBIG
