#include "orbitPath.h"

bool OrbitPath::Initialize(const Arguments& args, const std::string& texturePath, glm::vec2 orbitDistanceScaled, glm::vec2 orbitDistanceActual, glm::vec3 orbitalTiltNormal, int depth) {
	this->orbitalTiltNormal = orbitalTiltNormal;

	// Calculate how many radians apart each vertex should be
	float angleStep = glm::radians(360.0 / resolution);
	// Calculate how far along the UV each step should go
	float uvStep = 1.0 / resolution;

	float actualWidth = (depth < 2) ? width * 25 : width;

	// For each step of resolution
	for(int i = 0; i <= resolution; i++){
		// Calculate the inner and outer positions of a vertex at this step
		glm::vec3 outerPositionScaled = glm::vec3(cos(angleStep * i) * (orbitDistanceScaled.x + width), 0, sin(angleStep * i) * (orbitDistanceScaled.y + width));
		glm::vec3 innerPositionScaled = glm::vec3(cos(angleStep * i) * (orbitDistanceScaled.x - width), 0, sin(angleStep * i) * (orbitDistanceScaled.y - width));
		// And actual 

		glm::vec3 outerPositionActual = glm::vec3(cos(angleStep * i) * (orbitDistanceActual.x / 100000 + actualWidth), 0, sin(angleStep * i) * (orbitDistanceActual.y / 100000 + actualWidth));
		glm::vec3 innerPositionActual = glm::vec3(cos(angleStep * i) * (orbitDistanceActual.x / 100000 - actualWidth), 0, sin(angleStep * i) * (orbitDistanceActual.y / 100000 - actualWidth));

		// Add a vertex/index for the outer point (Scaled)
		Vertices.emplace_back(outerPositionScaled, glm::vec3(1), glm::vec2(0, uvStep * i));
		actualVertices.emplace_back(outerPositionActual, glm::vec3(1), glm::vec2(0, uvStep * i));
		Indices.push_back(i * 2 + 1);		

		// Add a vertex/index for the inner point (Scaled)
		Vertices.emplace_back(innerPositionScaled, glm::vec3(1), glm::vec2(1, uvStep * i));
		actualVertices.emplace_back(innerPositionActual, glm::vec3(1), glm::vec2(1, uvStep * i));
		Indices.push_back(i * 2);		
	}

	// Upload the vertecies and indices to the GPU
	FinalizeModel();
	// Add the data to the actual vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, actualVB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * actualVertices.size(), &actualVertices[0], GL_STATIC_DRAW);

	// Load the specified texture
	return LoadTextureFile(args, texturePath);
}

void OrbitPath::Update(unsigned int dt){
	// Tilt it
	model = rotateTo(glm::vec3(0, 1, 0), orbitalTiltNormal);

	// If our parent has a parent... set our position relative to our parent's parent
	if(getParent()->getParent())
		model = getParent()->getParent()->GetChildBaseModel() * model;
}

void OrbitPath::Render(GLint modelMatrix) {

	// Set the model matrix
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, glm::value_ptr(GetModel()));

	// Enable 3 vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Specify that we are using the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, (globalShouldScale ? VB : actualVB) );
	// Specify where in the vertex buffer we can find position, color, and UVs
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,uv));

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	// Specify that we are using the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Disable backface culling
	glDisable(GL_CULL_FACE);
	// Draw the triangles (The vertecies are laid out as a strip... so be sure to draw them as a strip)
	glDrawElements(GL_TRIANGLE_STRIP, Indices.size(), GL_UNSIGNED_INT, 0);

	// Disable the attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// Disable alpha blending
	glDisable(GL_BLEND);

	// Pass along to children.
	for(Object* child: children)
		child->Render(modelMatrix);
}
