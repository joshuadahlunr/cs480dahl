#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"
#include <SDL2/SDL.h>

// Class representing a camera
class Camera {
public:
	Camera();
	~Camera();
	bool Initialize(int w, int h);
	glm::mat4 GetProjection();
	glm::mat4 GetView();

	// The camera updates some matricies when the window is resized.
	static int windowResizeEventListener(void* data, SDL_Event* event);

private:
	glm::mat4 projection;
	glm::mat4 view;
};

#endif /* CAMERA_H */
