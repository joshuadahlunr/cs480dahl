#include "physics.h"
#include "engine.h"
#include "object.h"
#include "shader.h"
#include "camera.h"

// Backing/access for the singleton
Physics* Physics::singleton;
Physics* Physics::getSingleton() { return singleton; }

Physics::Physics(Object*& sceneRoot) : sceneRoot(sceneRoot) { }
Physics::~Physics() { }

bool Physics::Initialize(Engine* engine, const Arguments& args) {
	// Setup the singleton
	singleton = this;

	// Set physics world settings
	rp3d::PhysicsWorld::WorldSettings settings;
	settings.gravity = rp3d::Vector3(0, -2.0 / sqrt(5), -1.0 / sqrt(5)) * 9.81; // (0, -2, -1) normalized

	// Create the physics world
	world = factory.createPhysicsWorld(settings);
	world->setEventListener(this);

	// Debug Rendering
#ifdef PHYSICS_DEBUG
	// Enable debug in the world
	world->setIsDebugRenderingEnabled(true);

	// Generate the vertex buffer
	glGenBuffers(1, &debugLineBuffer);

	// Generate the shader
	debugShader = new Shader();
	debugShader->Initialize();
	debugShader->AddShader(GL_VERTEX_SHADER, "physics.vert.glsl", args);
	debugShader->AddShader(GL_FRAGMENT_SHADER, "physics.frag.glsl", args);
	debugShader->Finalize();
#endif

	return true;
}

void Physics::Update(float dt) {
	// Update the physics simulation
	world->update(dt);
}

void Physics::addContactCallback(Object* obj, ContactEvent e){ contactEvents[obj->getCollider().getEntity().id] = e; }

void Physics::onContact(const rp3d::CollisionCallback::CallbackData& callbackData) {
	for(int i = 0; i < callbackData.getNbContactPairs(); i++)
		if(callbackData.getContactPair(i).getNbContactPoints() > 0)
			for(auto idEvent: contactEvents)
				if(callbackData.getContactPair(i).getCollider1()->getEntity().id == idEvent.first || callbackData.getContactPair(i).getCollider2()->getEntity().id == idEvent.first)
					idEvent.second(callbackData.getContactPair(i));
}

#ifdef PHYSICS_DEBUG

// Function which convers a uint32 color into a glm::vec3
glm::vec3 fromUint(uint32_t color){
	glm::vec3 out;
	float a;

	a = float(0x000000ff & color) / 0xff;
	out.b = float(0x000000ff & (color >> 4)) / 0xff;
	out.g = float(0x000000ff & (color >> 8)) / 0xff;
	out.r = float(0x000000ff & (color >> 12)) / 0xff;
	return out;
}

void Physics::Render(Camera* camera) {
	// Get a reference to the debug renderer and set what we would like to see
	rp3d::DebugRenderer& debugRenderer = getWorld().getDebugRenderer();
	debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
	// debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_AABB, true);
	// debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, true);


	// Get the vertices for each debug triangle
	std::vector<Vertex> lineVerts;
	for(rp3d::DebugRenderer::DebugTriangle& tri: debugRenderer.getTriangles()){
		lineVerts.emplace_back(*((glm::vec3*)(&tri.point1)), fromUint(tri.color1), glm::vec2(0), glm::vec3(0));
		lineVerts.emplace_back(*((glm::vec3*)(&tri.point2)), fromUint(tri.color2), glm::vec2(0), glm::vec3(0));

		lineVerts.emplace_back(*((glm::vec3*)(&tri.point2)), fromUint(tri.color2), glm::vec2(0), glm::vec3(0));
		lineVerts.emplace_back(*((glm::vec3*)(&tri.point3)), fromUint(tri.color3), glm::vec2(0), glm::vec3(0));

		lineVerts.emplace_back(*((glm::vec3*)(&tri.point3)), fromUint(tri.color3), glm::vec2(0), glm::vec3(0));
		lineVerts.emplace_back(*((glm::vec3*)(&tri.point1)), fromUint(tri.color1), glm::vec2(0), glm::vec3(0));
	}

	// Get the vertices for each debug line
	for(rp3d::DebugRenderer::DebugLine& line: debugRenderer.getLines()){
		lineVerts.emplace_back(*((glm::vec3*)(&line.point1)), fromUint(line.color1), glm::vec2(0), glm::vec3(0));
		lineVerts.emplace_back(*((glm::vec3*)(&line.point2)), fromUint(line.color2), glm::vec2(0), glm::vec3(0));
	}

	// Upload the debug vertices to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, debugLineBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * lineVerts.size(), &lineVerts[0], GL_STATIC_DRAW);


	// Enable the physics debug shader
	debugShader->Enable();

	// Set the view and projection matrix
	glUniformMatrix4fv(debugShader->GetUniformLocation("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(camera->GetProjection()));
	glUniformMatrix4fv(debugShader->GetUniformLocation("viewMatrix"), 1, GL_FALSE, glm::value_ptr(camera->GetView()));

	// Disable depth testing
	glDepthMask(GL_FALSE);

	// Enable 3 vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// Specify where in the vertex buffer we can find position, color, and UVs
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,uv));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,normal));

	// Draw the lines
	glBindBuffer(GL_ARRAY_BUFFER, debugLineBuffer);
	glLineWidth(2.5); // Making sure they are wide enough to be sean
	glDrawArrays(GL_LINES, 0, lineVerts.size());

	// Disable the attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	// Enable depth testing
	glDepthMask(GL_TRUE);
}



#endif // PHYSICS_DEBIG
