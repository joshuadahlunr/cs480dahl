#include "object.h"

#include <fstream>
#include <sstream>

Object::Object() {
	// Create the vertex and face buffers for this object
	glGenBuffers(1, &VB);
	glGenBuffers(1, &IB);
}

Object::~Object() {
	// Make sure all of the children are freed
	for(Object* child: children){
		delete child;
		child = nullptr;
	}

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
	success &= LoadModelFile(filepath);

	// Initialize the children
	for(Object* child: children)
		success &= child->Initialize(args);

	return success;
}

bool Object::LoadModelFile(const std::string& path, glm::mat4 onImportTransformation){
	// Load the model
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

	// Error handling
	if(scene == nullptr){
		std::cerr << "Failed to import model `" << path << "`: ";
		std::cerr << importer.GetErrorString() << std::endl;
		return false;
	}

	// Variable which tracks the starting index in vertecies list of this mesh. That way if multiple models are loaded the indecies are correct for latter models
	int startingIndex = 0;
	// For each mesh...
	for(int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++){
		// Set the starting index to be the index after the current end of the vertecies array
		startingIndex = Vertices.size();

		const aiMesh* mesh = scene->mMeshes[meshIndex];
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

			// Add the vertex to the list of vertecies
			Vertices.emplace_back(/*position*/ glm::vec3(pos.x, pos.y, pos.z), color);
		}

		// For each face...
		for(int face = 0; face < mesh->mNumFaces; face++)
			// For each index in the face (3 in the triangles)
			for(int index = 0; index < 3; index++)
				// Add the index to the list of indecies
				Indices.push_back(mesh->mFaces[face].mIndices[index] + /* Make sure to compensate for multiple models */ startingIndex);
	}

	// Add the data to the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// Add the data to the face buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

	return true;
}

void Object::Update(unsigned int dt){
	// Pass along to children
	for(Object* child: children)
		child->Update(dt);
}

void Object::Render(GLint modelMatrix) {
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, glm::value_ptr(GetModel()));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

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
	model = (parent ? parent->model : glm::mat4(1)) * _model;
}
