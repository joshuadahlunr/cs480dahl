#include "rings.h"

bool Ring::Initialize(const Arguments& args, const std::string& texturePath) {
	// Calculate how many radians apart each vertex should be
	float angleStep = glm::radians(360.0 / resolution);
	// Calculate how far along the UV each step should go
	float uvStep = 1.0 / resolution;

	// For each step of resolution
	for(int i = 0; i <= resolution; i++){
		// Calculate the inner and outer positions of a vertex at this step
		glm::vec3 outerPosition = glm::vec3(cos(angleStep * i) * outerRadius, 0, sin(angleStep * i) * outerRadius);
		glm::vec3 innerPosition = glm::vec3(cos(angleStep * i) * innerRadius, 0, sin(angleStep * i) * innerRadius);

		// Add a vertex/index for the outer point
		Vertices.emplace_back(outerPosition, glm::vec3(1), glm::vec2(0, uvStep * i));
		Indices.push_back(i * 2 + 1);

		// Add a vertex/index for the inner point
		Vertices.emplace_back(innerPosition, glm::vec3(1), glm::vec2(1, uvStep * i));
		Indices.push_back(i * 2);
	}

	// Upload the vertecies and indices to the GPU
	FinalizeModel();

	// Load the specified texture
	return LoadTextureFile(args, texturePath);
}

void Ring::Update(unsigned int dt) {
	// Tilt the ring
	setModelRelativeToParent(rotateTo(glm::vec3(0, 1, 0), tilt));
}

void Ring::Render(GLint modelMatrix) {
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
