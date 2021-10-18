#ifndef SUBMESH_H
#define SUBMESH_H

#include "object.h"

// Objects which do nothing when initialized and link their model matrix to their parent each frame
// (used to hold data for models with multiple materials/submeshes)
class Submesh: public Object {
public:
	using Object::Object;

	// Submesh initialization doesn't do anything
	bool Initialize(const Arguments& args) override { return true; }
	// A submesh syncs it model matrix to its parent every frame
	void Update(unsigned int dt) override { setModelRelativeToParent(glm::mat4(1)); }
};

#endif /* end of include guard: SUBMESH_H */
