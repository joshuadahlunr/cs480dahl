#include "object.h"
#include "submesh.h"

#include <fstream>
#include <sstream>

// Object loading
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
	for(Object* child: children){
		delete child;
		child = nullptr;
		parent = nullptr;
	}

	// Clean up the lists of vertecies and indices
	Vertices.clear();
	Indices.clear();
}

bool Object::Initialize(const Arguments& args){
	bool success = true;

	// Get the showcase path from the arguments
	std::string filepath = args.getShowcaseModelPath();
	// If the filepath doesn't already have the shader directory path, add the shader dirrectory path
	std::string modelDirectory = args.getResourcePath() + "models/";
	if(filepath.find(modelDirectory) == std::string::npos)
		filepath = modelDirectory + filepath;

	// Load the model
	success &= LoadModelFile(args, filepath);

	// Initialize the children
	for(Object* child: children)
		success &= child->Initialize(args);

	return success;
}

bool Object::LoadModelFile(const Arguments& args, const std::string& path, glm::mat4 onImportTransformation){
	// Load the model
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

	// Error handling
	if(scene == nullptr){
		std::cerr << "Failed to import model `" << path << "`: ";
		std::cerr << importer.GetErrorString() << std::endl;
		return false;
	}

	// For each mesh...
	for(int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++){
		// First mesh is put in this object, future meshes are added as sub-object
		Object* obj;
		if(meshIndex == 0) obj = this;
		else{
			obj = new Submesh(); // Submesh's model matrix are linked to their parent
			obj->setParent(this);
		}

		// Extract this mesh from the scene
		const aiMesh* mesh = scene->mMeshes[meshIndex];

		// If the mesh has a material...
		if(mesh->mMaterialIndex > 0){
			// Extract the material from the scene
			const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			// Extract the path to the diffuse texture
			aiString _path;
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &_path);

			if(_path.length > 0)
				if( !obj->LoadTextureFile(args, std::string(_path.C_Str())) )
					return false;

		}

		// For each vertex...
		for(int vert = 0; vert < mesh->mNumVertices; vert++){

			// Extract the position
			auto _pos = mesh->mVertices[vert];
			// Apply an input transformation (defaults to the identity matrix)
			glm::vec4 pos(_pos.x, _pos.y, _pos.z, 1);
			pos = onImportTransformation * pos;

			// Extract the (first) vertex color if it exists
			glm::vec3 color(1, 1, 1); // White by default
			if(mesh->HasVertexColors(0)){
				auto col = mesh->mColors[0][vert];
				color = glm::vec3(col.r, col.g, col.b);
			}

			// Extract the (first) texture coordinates if they exist
			glm::vec2 uv(0, 0); // 0,0 by default
			if(mesh->HasTextureCoords(0)){
				auto tex = mesh->mTextureCoords[0][vert];
				uv = glm::vec2(tex.x, tex.y);
			}

			// Add the vertex to the list of vertecies
			obj->Vertices.emplace_back(/*position*/ glm::vec3(pos.x, pos.y, pos.z), color, uv);
		}

		// For each face...
		for(int face = 0; face < mesh->mNumFaces; face++)
			// For each index in the face (3 in the triangles)
			for(int index = 0; index < 3; index++)
				// Add the index to the list of indices
				obj->Indices.push_back(mesh->mFaces[face].mIndices[index]);

		// Add the data to the vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, obj->VB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * obj->Vertices.size(), &obj->Vertices[0], GL_STATIC_DRAW);

		// Add the data to the face buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->IB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * obj->Indices.size(), &obj->Indices[0], GL_STATIC_DRAW);
	}

	return true;
}

bool Object::LoadTextureFile(const Arguments& args, std::string path, bool makeRelative) {
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

	// Upload the imaage to the gpu
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Free the image
	stbi_image_free(img);

	return true;
}


void Object::Update(unsigned int dt){
	// Pass along to children
	for(Object* child: children)
		child->Update(dt);
}

void Object::Render(GLint modelMatrix) {
	// Set the model matrix
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, glm::value_ptr(GetModel()));

	// Enable 3 vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Specify that we are using the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	// Specify where in the vertex buffer we can find position, color, and UVs
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,uv));

	//bind texture (if it exists)
	if(tex != -1){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
	}

	// Specify that we are using the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	// Draw the triangles
	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

	// Disable the attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// Pass along to children.
	for(Object* child: children)
		child->Render(modelMatrix);
}

Object* Object::setParent(Object* p){
	// If the parent is the same as what we are setting it to... do nothing
	if(parent == p) return p;

	// If this object already has a parent... remove it as a child of that parent
	if(parent != nullptr)
		for(int i = 0; i < parent->children.size(); i++)
			if(parent->children[i] == this){ // TODO: Are pointer comparisons sufficient here?
				parent->children.erase(parent->children.begin() + i);
				break;
			}

	// Mark the new parent as our parent
	parent = p;
	// Add ourselves as a child of that parent
	p->addChild(this);
	return p;
}

Object* Object::addChild(Object* child){
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

void Object::Keyboard(const SDL_KeyboardEvent& e){
	// Pass along to children
	for(Object* child: children)
		child->Keyboard(e);
}

void Object::MouseButton(const SDL_MouseButtonEvent& e){
	// Pass along to children
	for(Object* child: children)
		child->MouseButton(e);
}

void Object::setModelRelativeToParent(glm::mat4 _model){
	// Multiply the new model by the parent's model (if we have a parent)
	childModel = model = (parent ? parent->childModel : glm::mat4(1)) * _model;
}

void Object::setChildModelRelativeToParent(glm::mat4 _model){
	// Multiply the new model by the parent's model (if we have a parent)
	childModel = (parent ? parent->childModel : glm::mat4(1)) * _model;
}
