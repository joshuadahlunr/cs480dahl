#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"
#include <SDL2/SDL.h>

class Camera {
public:
	Camera();
	~Camera();
	bool Initialize(int w, int h);
	glm::mat4 GetProjection();
	glm::mat4 GetView();

	static int windowResizeEventListener(void* data, SDL_Event* event);

private:
	glm::mat4 projection;
	glm::mat4 view;
};

#endif /* CAMERA_H */
