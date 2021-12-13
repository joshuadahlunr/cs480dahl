#include "object.h"
#include "shader.h"

#include <fstream>
#include <sstream>
#include <vector>

// Texture loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Model loading
#include <assimp/Importer.hpp>	//includes the importer, which is used to read our obj file
#include <assimp/scene.h>		//includes the aiScene object
#include <assimp/postprocess.h>	//includes the postprocessing variables for the importer
#include <assimp/color4.h>		//includes the aiColor4 object, which is used to handle the colors from the mesh objects

// Convex hull
#include "VHACD.h"

#include <BulletCollision/CollisionShapes/btConvexPointCloudShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>

Object::Object() {
	// Create the vertex and face buffers for this object
	glGenBuffers(1, &VB);
	glGenBuffers(1, &IB);
	// Mark that we don't have a parent
	parent = nullptr;
	// Ensure that we don't have a physics component
	rigidBody = nullptr;
}

Object::~Object() {
	// Make sure all of the children are freed
	for(auto& child: children)
		child.reset((Object*) nullptr);

	// Mark that we don't have a parent
	parent = nullptr;

	// Clean up the lists of vertecies and indices
	vertices.clear();
	indices.clear();

	// Destroy the rigid body (if it was initialized)
	if(rigidBody)
		Physics::getSingleton().getWorld()->removeRigidBody(rigidBody.get());
}

bool Object::initializeGraphics(const Arguments& args, std::string filepath, std::string texturePath) {
	bool success = true;
	// If the filepath doesn't already have the shader directory path, add the shader dirrectory path
	std::string modelDirectory = args.getResourcePath() + "models/";
	if(filepath.find(modelDirectory) == std::string::npos)
		filepath = modelDirectory + filepath;

	// Load the model
	success &= LoadModelFile(args, filepath);

	// Load the texture (error texture if none provided)
	success &= loadTextureFile(args, args.getResourcePath() + "textures/" + texturePath, false);

	// Ensure that the child model matrix is the same as the normal model matrix
	childModel = model;

	// initialize the children
	for(auto& child: children)
		success &= child->initializeGraphics(args);

	return success;
}

bool Object::initializePhysics(const Arguments& args, Physics& physics, int collisionGroup /*= 0*/, float mass /*= 1*/,  bool addToWorldAutomatically /*= true*/) {
	// Create a physics rigid body with the initial transform from the model matrix
	motionState = std::make_unique<btDefaultMotionState>();
	collisionShape = std::make_unique<btEmptyShape>();
	rigidBody = std::make_unique<btRigidBody>(mass, motionState.get(), collisionShape.get());

	// Add the new rigid body to the simulation (if we are adding it at this point)
	if(addToWorldAutomatically) addToPhysicsWorld(physics, collisionGroup);
	// Update the postion of the physics engine
	syncPhysicsWithGraphics();

	return true;
}

void Object::addToPhysicsWorld(Physics& physics, int collisionGroup /*= CollisionGroups::None*/){
	if(addedToPhysicsWorld) return;

	// Add the new rigid body to the simulation
	physics.getWorld()->addRigidBody(rigidBody.get(), collisionGroup, CollisionGroups::CG_ALL);
	addedToPhysicsWorld = true;
}

void Object::makeDynamic(bool recursive /*= true*/) {
	rigidBody->setCollisionFlags( rigidBody->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT & ~btCollisionObject::CF_KINEMATIC_OBJECT );  
	// rigidBody->setActivationState(ACTIVE_TAG);

	// Recursively update the children if requested
	if(recursive) for(auto& child: children) child->makeDynamic(true);
}
void Object::makeStatic(bool recursive /*= true*/) {
	rigidBody->setCollisionFlags( (rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT) & ~btCollisionObject::CF_KINEMATIC_OBJECT ); 
	// rigidBody->setActivationState(ACTIVE_TAG);

	// Recursively update the children if requested
	if(recursive) for(auto& child: children) child->makeStatic(true);
}
void Object::makeKinematic(bool recursive /*= true*/) {
	rigidBody->setCollisionFlags( (rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT) & ~btCollisionObject::CF_STATIC_OBJECT); 
	rigidBody->setActivationState(DISABLE_DEACTIVATION);

	// Recursively update the children if requested
	if(recursive) for(auto& child: children) child->makeKinematic(true);
}

void Object::createCapsuleCollider(float radius, float height) {
	// Specify the shape
	collisionShape = std::make_unique<btCapsuleShape>(radius, height);

	// Add the collider to the rigid/static body
	rigidBody->setCollisionShape(collisionShape.get());
}

void Object::createBoxCollider(glm::vec3 halfExtents) {
	// Specify the shape
	collisionShape = std::make_unique<btBoxShape>( toBullet(halfExtents) );

	// Add the collider to the rigid/static body
	rigidBody->setCollisionShape(collisionShape.get());
}

void Object::createSphereCollider(float radius) {
	// Specify the shape
	collisionShape = std::make_unique<btSphereShape>( radius );

	// Add the collider to the rigid/static body
	rigidBody->setCollisionShape(collisionShape.get());
}

bool Object::createMeshCollider(const Arguments& args, Physics& physics, size_t maxHulls /*= 32*/, std::string path /*= ""*/) {
	std::vector<float> points;
	std::vector<int> indices;

	// If a model to load was specified, load it into the collision mesh
	if (path != "") {
		std::string modelDirectory = args.getResourcePath() + "models/";
		if(path.find(modelDirectory) == std::string::npos)
			path = modelDirectory + path;
		//std::cout << path << std::endl;
		// Load the model
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

		// Error handling
		if(scene == nullptr) {
			std::cerr << "Failed to import model `" << path << "`: ";
			std::cerr << importer.GetErrorString() << std::endl;
			return false;
		}

		// For each mesh...
		for(int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {

			std::vector<glm::vec3> tempVertices = std::vector<glm::vec3>();

			// Extract this mesh from the scene
			const aiMesh* mesh = scene->mMeshes[meshIndex];

			// For each vertex...
			for(int vert = 0; vert < mesh->mNumVertices; vert++) {

				// Extract the position
				auto _pos = mesh->mVertices[vert];

				// Add the vertex to the list of vertecies
				points.push_back(_pos.x);
				points.push_back(_pos.y);
				points.push_back(_pos.z);
			}

			// For each face...
			for(int face = 0; face < mesh->mNumFaces; face++)
				// For each index in the face (3 in the triangles)
				for(int index = 0; index < 3; index++)
					// Add the index to the list of indices
					indices.push_back(mesh->mFaces[face].mIndices[index]);

			// Store the concave mesh using the exact model data
			for(glm::vec3& vert: tempVertices) {
				points.push_back(vert.x);
				points.push_back(vert.y);
				points.push_back(vert.z);
			}
		}

	// If we aren't loading a new mesh... our collision mesh is just the graphics mesh
	} else {
		for(Vertex& vert: vertices) {
			points.push_back(vert.vertex.x);
			points.push_back(vert.vertex.y);
			points.push_back(vert.vertex.z);
		}
		indices = std::vector<int>(this->indices.begin(), this->indices.end());
	}


	// If we should convert the data into a convex mesh... perform a convex decomposition
	if (maxHulls > 0) {
		VHACD::IVHACD::Parameters params; // V-HACD parameters
		// params.m_maxNumVerticesPerCH = 128;
		params.m_maxConvexHulls = maxHulls;
		VHACD::IVHACD* interfaceVHACD = VHACD::CreateVHACD(); // Create Decomposer

		// Compute approximate convex decomposition
		auto uint32Indecies = std::vector<uint32_t>(indices.begin(), indices.end());
		// bool res = interfaceVHACD->Compute(points.data(), points.size(), uint32Indecies.data(), uint32Indecies.size() / 3, params);
		bool res = interfaceVHACD->Compute(points.data(), points.size() / 3, uint32Indecies.data(), uint32Indecies.size() / 3, params);

		collisionShape = std::make_unique<btCompoundShape>();

		// Pass each computed convex hull off to react
		size_t nConvexHulls = interfaceVHACD->GetNConvexHulls(); // Get the number of convex-hulls
		for (size_t i = 0; i < nConvexHulls; i++) {
			// Get the i-th convex-hull
			VHACD::IVHACD::ConvexHull hull;
			interfaceVHACD->GetConvexHull(i, hull); 

			// Save the points of the hull
			auto verts = std::vector<btVector3>();
			for (size_t v = 0, idx = 0; v < hull.m_nPoints; ++v, idx+=3)
				verts.emplace_back(hull.m_points[idx], hull.m_points[idx + 1], hull.m_points[idx + 2]);
			
			// Save the indecies of the hull
			auto indices = std::vector<int>();
			for (size_t t = 0, idx = 0; t < hull.m_nTriangles; ++t, idx +=3){
				indices.push_back(hull.m_triangles[idx]);
				indices.push_back(hull.m_triangles[idx + 1]);
				indices.push_back(hull.m_triangles[idx + 2]);
			}

			trimeshs.emplace_back( std::move(std::make_unique<btTriangleMesh>()) );
			for (int i = 0; i < indices.size(); i += 3)
			{
				int index0 = indices[i];
				int index1 = indices[i + 1];
				int index2 = indices[i + 2];

				btVector3 vertex0 = verts[index0];
				btVector3 vertex1 = verts[index1];
				btVector3 vertex2 = verts[index2];

				trimeshs.back()->addTriangle(vertex0, vertex1, vertex2);
			}

			shapes.emplace_back( std::move(std::make_unique<btConvexTriangleMeshShape>( trimeshs.back().get() )) );
			((btCompoundShape*) collisionShape.get())->addChildShape(btTransform::getIdentity(), shapes.back().get());
		}

		// Release decomposer memory
		interfaceVHACD->Clean();
		interfaceVHACD->Release(); 

	// Us a concave mesh
	} else {//if(maxHulls == 1) 
		trimeshs.emplace_back( std::move(std::make_unique<btTriangleMesh>()) );
		for (int i = 0; i < indices.size(); i += 3)
		{
			int index0 = indices[i];
			int index1 = indices[i + 1];
			int index2 = indices[i + 2];

			btVector3 vertex0 = toBullet(vertices[index0].vertex);
			btVector3 vertex1 = toBullet(vertices[index1].vertex);
			btVector3 vertex2 = toBullet(vertices[index2].vertex);

			trimeshs.back()->addTriangle(vertex0, vertex1, vertex2);
		}

		collisionShape  = std::make_unique<btBvhTriangleMeshShape>(trimeshs.back().get(), true);
	}

	// Add the collider to the rigid/static body
	rigidBody->setCollisionShape(collisionShape.get());

	return true;
}

bool Object::LoadModelFile(const Arguments& args, const std::string& path, glm::mat4 onImportTransformation) {
	// Load the model
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

	// Error handling
	if(scene == nullptr) {
		std::cerr << "Failed to import model `" << path << "`: ";
		std::cerr << importer.GetErrorString() << std::endl;
		return false;
	}

	// For each mesh...
	for(int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
		// First mesh is put in this object, future meshes are added as sub-object
		Object::ptr obj;
		if(meshIndex == 0) obj = shared_from_this();
		else{
			obj = std::make_shared<Submesh>(); // Submesh's model matrix are linked to their parent
			obj->setParent(shared_from_this());
		}

		// Remove a previous model
		obj->vertices.clear();
		obj->indices.clear();

		// Extract this mesh from the scene
		const aiMesh* mesh = scene->mMeshes[meshIndex];

		// If the mesh has a material...
		if(mesh->mMaterialIndex > 0) {
			// Extract the material from the scene
			const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			// Extract the path to the diffuse texture
			aiString _path;
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &_path);

			if(_path.length > 0)
				if( !obj->loadTextureFile(args, std::string(_path.C_Str())) )
					return false;

		}

		// For each vertex...
		for(int vert = 0; vert < mesh->mNumVertices; vert++) {

			// Extract the position
			auto _pos = mesh->mVertices[vert];
			// Apply an input transformation (defaults to the identity matrix)
			glm::vec4 pos(_pos.x, _pos.y, _pos.z, 1);
			pos = onImportTransformation * pos;

			// Extract the (first) vertex color if it exists
			glm::vec3 color(1, 0, 0); // Enable textures, no voxel tint by default
			if(mesh->HasVertexColors(0)) {
				auto col = mesh->mColors[0][vert];
				color = glm::vec3(col.r, col.g, col.b);
			}

			// Extract the (first) texture coordinates if they exist
			glm::vec2 uv(0, 0); // 0,0 by default
			if(mesh->HasTextureCoords(0)) {
				auto tex = mesh->mTextureCoords[0][vert];
				uv = glm::vec2(tex.x, -tex.y);
			}

			// Extract the (first) texture coordinates if they exist
			glm::vec3 normal(0, 0, 0); // 0,0,0 by default
			if(mesh->HasNormals()) {
				auto tex = mesh->mNormals[vert];
				normal = glm::vec3(tex.x, tex.y, tex.z);
			}

			// Add the vertex to the list of vertecies
			obj->vertices.emplace_back(/*position*/ glm::vec3(pos.x, pos.y, pos.z), color, uv, normal);
		}

		// For each face...
		for(int face = 0; face < mesh->mNumFaces; face++)
			// For each index in the face (3 in the triangles)
			for(int index = 0; index < 3; index++)
				// Add the index to the list of indices
				obj->indices.push_back(mesh->mFaces[face].mIndices[index]);

		// Upload the model to the GPU
		obj->finalizeModel();
	}

	return true;
}

// Uploads the model data to the GPU
void Object::finalizeModel() {
	// Add the data to the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// Add the data to the face buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

// Function which loads a texture for this model and binds it
bool Object::loadTextureFile(const Arguments& args, std::string path, bool makeRelative) {
	// Make the extracted path relative
	std::string modelDirectory = args.getResourcePath() + "models/";
	if(path.find(modelDirectory) == std::string::npos && makeRelative)
		path = modelDirectory + path;

	// Load the image
	int width, height, channelsPresent;
	unsigned char* img = stbi_load(path.c_str(), &width, &height, &channelsPresent, /*RGBA*/ 4);
	if(img == nullptr) {
		std::cerr << "Failed to load image `" << path << "`" << std::endl;
		return false;
	}

	// Upload the image to the gpu
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Free the image
	stbi_image_free(img);

	return true;
}


void Object::update(float dt) {
	// Make sure the graphics position is updated to match the physics position
	syncGraphicsWithPhysics();

	// Pass along to children
	for(auto& child: children)
		child->update(dt);
}

void Object::render(Shader* boundShader) {
	// Set the model matrix
	glUniformMatrix4fv(boundShader->getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(getModel()));

	// Enable 3 vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// Specify that we are using the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	// Specify where in the vertex buffer we can find position, color, and UVs
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,uv));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,normal));


	//bind texture (if it exists)
	if(tex != -1) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
	}

	// Specify that we are using the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	// Enable backface culling
	glEnable(GL_CULL_FACE);
	// Draw the triangles
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// Disable the attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	// Pass along to children.
	for(auto& child: children)
		child->render(boundShader);
}

Object::ptr Object::setParent(Object::ptr p) {
	// If the parent is the same as what we are setting it to... do nothing
	if(parent == p.get()) return p;

	// If this object already has a parent... remove it as a child of that parent
	if(parent != nullptr)
		for(int i = 0; i < parent->children.size(); i++)
			if(parent->children[i].get() == this) { // TODO: Are pointer comparisons sufficient here?
				parent->children.erase(parent->children.begin() + i);
				break;
			}

	// Update the depth to be one more than the depth of the parent
	uint* pSceneDepth = (uint*) &sceneDepth;
	if(p) *pSceneDepth = p->sceneDepth + 1;
	else *pSceneDepth = 0;

	// Mark the new parent as our parent
	parent = p.get();
	// Add ourselves as a child of that parent
	p->addChild(shared_from_this());
	return p;
}

Object::ptr Object::addChild(Object::ptr child) {
	// If the object is already a child... don't bother adding
	for(auto& c: children)
		if(c == child)
			return child;

	// Add the object as a child
	children.push_back(child);
	// Mark us as the object's parent
	child->setParent(shared_from_this());

	return child;
}

void Object::keyboard(const SDL_KeyboardEvent& e) {
	// Pass along to children
	for(auto& child: children)
		child->keyboard(e);
}

void Object::mouseButton(const SDL_MouseButtonEvent& e) {
	// Pass along to children
	for(auto& child: children)
		child->mouseButton(e);
}

void Object::setModel(glm::mat4 _model) {
	childModel = model = _model;
	syncPhysicsWithGraphics();
}

void Object::setModelRelativeToParent(glm::mat4 _model) {
	// Multiply the new model by the parent's model (if we have a parent)
	childModel = model = (parent ? parent->childModel : glm::mat4(1)) * _model;
	// Sync the physics simulation
	syncPhysicsWithGraphics();
}

void Object::setChildModelRelativeToParent(glm::mat4 _model) {
	// Multiply the new model by the parent's model (if we have a parent)
	childModel = (parent ? parent->childModel : glm::mat4(1)) * _model;
}

void Object::setPosition(glm::vec3 _pos, bool relativeToParent /*= false*/) {
	// If the position should be relative to parent multiply it by the parent's child base model
	if(relativeToParent)
		_pos = glm::vec3(getParent()->getChildBaseModel() * glm::vec4(_pos, 1));

	model[3][0] = _pos[0]; model[3][1] = _pos[1]; model[3][2] = _pos[2];
	setModel(model);
}

void Object::setRotation(glm::quat rot, bool relativeToParent /*= false*/){
	glm::vec3 translate, scale;
	glm::quat oldRot;
	decomposeModelMatrix(translate, oldRot, scale);

	if(relativeToParent)
		rot = glm::quat_cast((getParent()->getChildBaseModel() * glm::mat4_cast(rot))); // TODO: need to use the inverse transpose of base model?

	setModel(constructMat4(translate, rot, scale));
}

glm::quat Object::getRotation() {
	glm::vec3 translate, scale;
	glm::quat rotate;
	decomposeModelMatrix(translate, rotate, scale);
	return rotate;
}

// TODO: scale not working?
void Object::setScale(glm::vec3 scale, bool relativeToParent /*= false*/){
	glm::vec3 translate, oldScale;
	glm::quat rot;

	if(relativeToParent){
		getParent()->decomposeChildBaseModelMatrix(translate, rot, oldScale);
		scale = glm::vec3(glm::scale(glm::mat4(1), oldScale) * glm::vec4(scale, 0)); // TODO: need to use the inverse transpose of scale matrix?
	}

	decomposeModelMatrix(translate, rot, oldScale);
	setModel(constructMat4(translate, rot, scale));
}

glm::vec3 Object::getScale(){
	glm::vec3 translate, scale;
	glm::quat rotate;
	decomposeModelMatrix(translate, rotate, scale);
	return scale;
}
