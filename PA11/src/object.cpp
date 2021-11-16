#include "object.h"
#include "shader.h"

#include <fstream>
#include <sstream>
#include <vector>
// Convex hull
#include <quickhull/QuickHull.hpp>

// Texture loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Model loading
#include <assimp/Importer.hpp>	//includes the importer, which is used to read our obj file
#include <assimp/scene.h>		//includes the aiScene object
#include <assimp/postprocess.h>	//includes the postprocessing variables for the importer
#include <assimp/color4.h>		//includes the aiColor4 object, which is used to handle the colors from the mesh objects

Object::Object() {
	// Create the vertex and face buffers for this object
	glGenBuffers(1, &VB);
	glGenBuffers(1, &IB);
	// Mark that we don't have a parent
	parent = nullptr;
}

Object::~Object() {
	// Make sure all of the children are freed
	for(Object* child: children) {
		delete child;
		child = nullptr;
		parent = nullptr;
	}

	// Clean up the lists of vertecies and indices
	vertices.clear();
	indices.clear();
	collisionMesh.clear();

	// Destroy the rigid body (if it was initialized)
	if(rigidBody)
		Physics::getSingleton()->getWorld().destroyRigidBody(rigidBody);
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
	for(Object* child: children)
		success &= child->initializeGraphics(args);

	return success;
}

bool Object::initializePhysics(const Arguments& args, Physics& physics, bool _static) {
	// Create a physics rigid body with the initial transform from the model matrix
	rigidBody = Physics::getSingleton()->getWorld().createRigidBody(getGraphicsTransform());

	// Set wether the object is static or dynamic
	if(_static) rigidBody->setType(rp3d::BodyType::STATIC);
	else rigidBody->setType(rp3d::BodyType::DYNAMIC);

	return true;
}

rp3d::Transform Object::getPhysicsTransform() {
	return rigidBody->getTransform();
}

rp3d::Transform Object::getGraphicsTransform() {
	rp3d::Transform transform;
	transform.setFromOpenGL(glm::value_ptr(model));
	return transform;
}

void Object::addCapsuleCollider(float radius, float height, rp3d::Transform transform /*= rp3d::Transform()*/) {
	// Specify the shape
	rp3d::CollisionShape* shape = Physics::getSingleton()->getFactory().createCapsuleShape(radius, height);

	// Add the collider to the rigid/static body
	collider = rigidBody->addCollider(shape, transform);
}

void Object::addBoxCollider(glm::vec3 halfExtents, rp3d::Transform transform /*= rp3d::Transform()*/) {
	// Specify the shape
	rp3d::CollisionShape* shape = Physics::getSingleton()->getFactory().createBoxShape(*((rp3d::Vector3*)&halfExtents));

	// Add the collider to the rigid/static body
	collider = rigidBody->addCollider(shape, transform);
}

void Object::addSphereCollider(float radius, rp3d::Transform transform /*= rp3d::Transform()*/) {
	// Specify the shape
	rp3d::CollisionShape* shape = Physics::getSingleton()->getFactory().createSphereShape(radius);

	// Add the collider to the rigid/static body
	collider = rigidBody->addCollider(shape, transform);
}

bool Object::addMeshCollider(const Arguments& args, bool makeConvex /*= true*/, rp3d::Transform transform /*= rp3d::Transform()*/, std::string path) {
	// Create new collision mesh and shape
	collisionMesh = CollisionMesh();
	rp3d::CollisionShape* shape;

	//std::cout << "Adding Mesh Collider: " << path << std::endl;

	// Store memory for collision meshs because they need to exist for the life of the object
	if (makeConvex) {
		// store vertex positions temporarily
		std::vector<quickhull::Vector3<float>> positions;
		for(Vertex& vert: vertices) {
			positions.push_back(quickhull::Vector3<float>(vert.vertex.x, vert.vertex.y, vert.vertex.z));
		}

		// create hull concave with position data
		quickhull::QuickHull<float> qh;
		auto hull = qh.getConvexHull(&positions[0], positions.size(), false, false);

		// store the concave mesh using the hull data
		collisionMesh.numVertices = hull.getVertexBuffer().size();
		// std::cout << positions.size() << std::endl;
		// std::cout << collisionMesh.numVertices << std::endl;
		// std::cout << hull.getIndexBuffer().size() << std::endl;

		collisionMesh.vertexData = new float[collisionMesh.numVertices * 3];
		int i = 0;
		for(quickhull::Vector3<float> vec: hull.getVertexBuffer()) {
			collisionMesh.vertexData[i] = vec.x;
			collisionMesh.vertexData[i + 1] = vec.y;
			collisionMesh.vertexData[i + 2] = vec.z;
			i += 3;
		}
		collisionMesh.indiceData = new int[hull.getIndexBuffer().size()];
		for (int i = 0; i < hull.getIndexBuffer().size(); i++) {
			collisionMesh.indiceData[i] = hull.getIndexBuffer()[i];
		}
	} else {
		// if mesh collider specified
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
				std::vector<int> tempIndices = std::vector<int>();

				// Extract this mesh from the scene
				const aiMesh* mesh = scene->mMeshes[meshIndex];

				// For each vertex...
				for(int vert = 0; vert < mesh->mNumVertices; vert++) {

					// Extract the position
					auto _pos = mesh->mVertices[vert];

					// Add the vertex to the list of vertecies
					tempVertices.emplace_back(/*position*/ glm::vec3(_pos.x, _pos.y, _pos.z));
				}

				// For each face...
				for(int face = 0; face < mesh->mNumFaces; face++)
					// For each index in the face (3 in the triangles)
					for(int index = 0; index < 3; index++)
						// Add the index to the list of indices
						tempIndices.push_back(mesh->mFaces[face].mIndices[index]);

				// store the concave mesh using the exact model data
				collisionMesh.numVertices = tempVertices.size();
				collisionMesh.vertexData = new float[collisionMesh.numVertices * 3];
				int i = 0;
				for(glm::vec3& vert: tempVertices) {
					collisionMesh.vertexData[i] = vert.x;
					collisionMesh.vertexData[i + 1] = vert.y;
					collisionMesh.vertexData[i + 2] = vert.z;
					i += 3;
				}
				collisionMesh.indiceData = new int[tempIndices.size()];
				for(int i = 0; i < tempIndices.size(); ++i)
					collisionMesh.indiceData[i] = tempIndices[i];

				//std::cout << tempVertices.size() << " " << tempIndices.size() << std::endl;
			}
		} else { // if not convex and mesh from file
			// store the concave mesh using the exact model data
			collisionMesh.numVertices = vertices.size();
			collisionMesh.vertexData = new float[collisionMesh.numVertices * 3];
			int i = 0;
			for(Vertex& vert: vertices) {
				collisionMesh.vertexData[i] = vert.vertex.x;
				collisionMesh.vertexData[i + 1] = vert.vertex.y;
				collisionMesh.vertexData[i + 2] = vert.vertex.z;
				i += 3;
			}
			collisionMesh.indiceData = new int[indices.size()];
			for(int i = 0; i < indices.size(); ++i)
				collisionMesh.indiceData[i] = indices[i];
		}
	}

	// Create triangle array
	rp3d::TriangleVertexArray* triangleArray = new rp3d::TriangleVertexArray(
		collisionMesh.numVertices * 3, // size of vertex data
		collisionMesh.vertexData, // start of vertex data
		3 * sizeof(float), // size of one vertex
		collisionMesh.numVertices / 3, // size of indice data
		collisionMesh.indiceData, // start of indice data
		3 * sizeof(int), // size of one triangle
		rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
		rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

	rp3d::TriangleMesh* triangleMesh = Physics::getSingleton()->getFactory().createTriangleMesh();

	// Add the triangle vertex array to the triangle mesh
	triangleMesh->addSubpart(triangleArray);

	// Create the concave mesh shape
	shape = Physics::getSingleton()->getFactory().createConcaveMeshShape(triangleMesh);

	// Add the collider to the rigid/static body
	collider = rigidBody->addCollider(shape, transform);
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
		Object* obj;
		if(meshIndex == 0) obj = this;
		else{
			obj = new Submesh(); // Submesh's model matrix are linked to their parent
			obj->setParent(this);
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
			glm::vec3 color(1, 1, 1); // White by default
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
	// Get Rigidbody updated position (if it exists)
	if(rigidBody) {
		rigidBody->getTransform().getOpenGLMatrix(glm::value_ptr(model));
		childModel = model;
	}

	// Pass along to children
	for(Object* child: children)
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
	for(Object* child: children)
		child->render(boundShader);
}

Object* Object::setParent(Object* p) {
	// If the parent is the same as what we are setting it to... do nothing
	if(parent == p) return p;

	// If this object already has a parent... remove it as a child of that parent
	if(parent != nullptr)
		for(int i = 0; i < parent->children.size(); i++)
			if(parent->children[i] == this) { // TODO: Are pointer comparisons sufficient here?
				parent->children.erase(parent->children.begin() + i);
				break;
			}

	// Update the depth to be one more than the depth of the parent
	if(p) sceneDepth = p->sceneDepth + 1;
	else sceneDepth = 0;

	// Mark the new parent as our parent
	parent = p;
	// Add ourselves as a child of that parent
	p->addChild(this);
	return p;
}

Object* Object::addChild(Object* child) {
	// If the object is already a child... don't bother adding
	for(Object* c: children)
		if(c == child)
			return child;

	// Add the object as a child
	children.push_back(child);
	// Mark us as the object's parent
	child->setParent(this);

	return child;
}

void Object::keyboard(const SDL_KeyboardEvent& e) {
	// Pass along to children
	for(Object* child: children)
		child->keyboard(e);
}

void Object::mouseButton(const SDL_MouseButtonEvent& e) {
	// Pass along to children
	for(Object* child: children)
		child->mouseButton(e);
}

void Object::setModelRelativeToParent(glm::mat4 _model) {
	// Multiply the new model by the parent's model (if we have a parent)
	childModel = model = (parent ? parent->childModel : glm::mat4(1)) * _model;
}

void Object::setChildModelRelativeToParent(glm::mat4 _model) {
	// Multiply the new model by the parent's model (if we have a parent)
	childModel = (parent ? parent->childModel : glm::mat4(1)) * _model;
}
