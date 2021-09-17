#include "object.h"

#include <fstream>
#include <sstream>

Object::Object() {
	// Create the vertex and index buffers for this object
	glGenBuffers(1, &VB);
	glGenBuffers(1, &IB);
}

Object::~Object() {
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
	success &= LoadOBJFile(filepath);

	// Initialize the children
	for(Object* child: children)
		success &= child->Initialize(args);

	return success;
}

bool Object::LoadOBJFile(const std::string& path, glm::mat4 onImportTransformation){
	std::ifstream objFile(path);
	if(!objFile){
		std::cerr << "Model `" << path << "` not found!" << std::endl;
		return false; // If the file doesn't exist then there is an issue
	}

	std::string line;
	while(objFile){
		std::getline(objFile, line);
		size_t lineStart = line.find_first_not_of(" \t\n\r");

		// Variable unessicary characters can be read into
		char trash;

		// Ignore empty lines
		if(line.empty())
			continue;

		// Ignore comment lines
		if(line[lineStart] == '#')
			continue;

		// Parse the vertecies
		if(line.substr(lineStart, lineStart + 2) == "v "){
			std::stringstream s(line.substr(lineStart + 1));

			// Parse the position
			glm::vec3 pos;
			s >> pos.x >> pos.y >> pos.z;

			// Apply the import transformation to the position
			glm::vec4 _pos;
			_pos.x = pos.x; _pos.y = pos.y; _pos.z = pos.z; _pos.w = 1; // _pos = pos
			_pos = onImportTransformation * _pos;
			pos.x = _pos.x; pos.y = _pos.y; pos.z = _pos.z; // pos = _pos

			// Parse the vertex color (may not be present so check that we are still good after each read)
			glm::vec3 color;
			if(s) s >> color.r;
			if(s) s >> color.g;
			if(s) s >> color.b;

			Vertices.emplace_back(pos, color);
		}

		// Parse the indecies
		if(line.substr(lineStart, lineStart + 2) == "f "){
			std::stringstream s(line.substr(lineStart + 1));

			std::string part;
			// We are assuming three indecies per face
			for(int i = 0; i < 3; i++){
				if(s) s >> part;
				else {  // If we don't have at least three vertecies there is a problem with the file
					std::cerr << "Model face `" << line << "` contains too few vertecies!" << std::endl;
					return false;
				}
				std::stringstream partStream(part);

				int vert, texture, normal;

				partStream >> vert; // Vertex isn't optional
				// Optional texture
				if(partStream) partStream >> trash; // remove /
				if(partStream) {
					partStream >> trash;
					// If there is a number in the middle, read it into texture
					if(isdigit(trash) || trash == '-'){
						partStream.putback(trash);
						partStream >> texture;
					// Otherwise there is no texture (vert//normal)
					} else
						partStream.putback(trash);
				}
				// Optional normal
				if(partStream) partStream >> trash; // Remove /
				if(partStream) partStream >> normal;

				// If the vertex is negative... then it is based on the end of the vertex array instead of the beginning
				if(vert < 0) vert = Vertices.size() + vert;
				// Same thing for texture and normal

				Indices.push_back(vert);
			}

			if(s) s >> part;
			if(s) {	// If we have more than 3 vertecies there is a problem with file
				std::cerr << "Model face `" << line << "` contains too many vertecies!" << std::endl;
				return false;
			}
		}

		// std::cout << line << std::endl;
	}

	// The index works at a 0th index
	for(unsigned int i = 0; i < Indices.size(); i++)
		Indices[i] = Indices[i] - 1;

	// Add the data to the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// Add the data to the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

	return true;
}

void Object::Update(unsigned int dt){
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
	if(parent == p) return p;

	if(parent != nullptr)
		for(int i = 0; i < parent->children.size(); i++)
			if(parent->children[i] == this){ // TODO: Are pointer comparisons sufficient here?
				parent->children.erase(parent->children.begin() + i);
				break;
			}

	parent = p;
	p->addChild(this);
	return p;
}

Object* Object::addChild(Object* child){
	// If the object is already a child... don't bother adding
	for(Object* c: children)
		if(c == child)
			return child;

	children.push_back(child);
	child->setParent(this);
	return child;
}

void Object::Keyboard(const SDL_KeyboardEvent& e){
	for(Object* child: children)
		child->Keyboard(e);
}

void Object::MouseButton(const SDL_MouseButtonEvent& e){
	for(Object* child: children)
		child->MouseButton(e);
}

void Object::setModelRelativeToParent(glm::mat4 _model){
	model = (parent ? parent->model : glm::mat4(1)) * _model;
}
