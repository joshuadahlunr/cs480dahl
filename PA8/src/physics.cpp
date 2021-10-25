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

	// Create the physics world
	world = factory.createPhysicsWorld();
	
	
	// world->setIsDebugRenderingEnabled(true); 

	// glGenBuffers(1, &debugLineBuffer);

	// debugShader = new Shader();
	// debugShader->Initialize();
	// debugShader->AddShader(GL_VERTEX_SHADER, "physics.vert.glsl", args);
	// debugShader->AddShader(GL_FRAGMENT_SHADER, "physics.frag.glsl", args);
	// debugShader->Finalize();

	return true;
}

void Physics::Update(unsigned int dt) {
	// Update the physics simulation
	world->update(dt * 0.001);
}

glm::vec3 fromUint(uint32_t color){
	glm::vec3 out;
	float a;

	a = (0x000000ff & color) / 256;
	out.b = (0x000000ff & (color >> 4)) / 256; 
	out.g = (0x000000ff & (color >> 8)) / 256; 
	out.r = (0x000000ff & (color >> 12)) / 256; 
	return out;
}

// void Physics::Render(Camera* camera) {
// 	rp3d::DebugRenderer& debugRenderer = getWorld().getDebugRenderer(); 
// 	debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
// 	// debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_AABB, true);
// 	// debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, true);


// 	// Get the vertices for each debug triangle
// 	std::vector<Vertex> lineVerts;
// 	for(rp3d::DebugRenderer::DebugTriangle& tri: debugRenderer.getTriangles()){
// 		lineVerts.emplace_back(*((glm::vec3*)(&tri.point1)), fromUint(tri.color1), glm::vec2(0));
// 		lineVerts.emplace_back(*((glm::vec3*)(&tri.point2)), fromUint(tri.color2), glm::vec2(0));

// 		lineVerts.emplace_back(*((glm::vec3*)(&tri.point2)), fromUint(tri.color2), glm::vec2(0));
// 		lineVerts.emplace_back(*((glm::vec3*)(&tri.point3)), fromUint(tri.color3), glm::vec2(0));

// 		lineVerts.emplace_back(*((glm::vec3*)(&tri.point3)), fromUint(tri.color3), glm::vec2(0));
// 		lineVerts.emplace_back(*((glm::vec3*)(&tri.point1)), fromUint(tri.color1), glm::vec2(0));
// 	}

// 	// Get the vertices for each debug line
// 	for(rp3d::DebugRenderer::DebugLine& line: debugRenderer.getLines()){
// 		lineVerts.emplace_back(*((glm::vec3*)(&line.point1)), fromUint(line.color1), glm::vec2(0));
// 		lineVerts.emplace_back(*((glm::vec3*)(&line.point2)), fromUint(line.color2), glm::vec2(0));
// 	}

// 	glBindBuffer(GL_ARRAY_BUFFER, debugLineBuffer);
// 	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * lineVerts.size(), &lineVerts[0], GL_STATIC_DRAW);


// 	// Enable the skybox shader
// 	debugShader->Enable();

// 	// Set the view and projection matrix
// 	glUniformMatrix4fv(debugShader->GetUniformLocation("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(camera->GetProjection()));
// 	glUniformMatrix4fv(debugShader->GetUniformLocation("viewMatrix"), 1, GL_FALSE, glm::value_ptr(camera->GetView()));

// 	// Disable depth testing
// 	glDepthMask(GL_FALSE);

// 	// Enable 3 vertex attributes
// 	glEnableVertexAttribArray(0);
// 	glEnableVertexAttribArray(1);
// 	glEnableVertexAttribArray(2);

// 	// Specify where in the vertex buffer we can find position, color, and UVs
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
// 	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));
// 	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,uv));

// 	// Draw the lines
// 	glBindBuffer(GL_ARRAY_BUFFER, debugLineBuffer);
// 	glLineWidth(5);
// 	glDrawArrays(GL_LINES, 0, lineVerts.size());

// 	// Disable the attributes
// 	glDisableVertexAttribArray(0);
// 	glDisableVertexAttribArray(1);
// 	glDisableVertexAttribArray(2);

// 	// Enable depth testing
// 	glDepthMask(GL_TRUE);
// }
