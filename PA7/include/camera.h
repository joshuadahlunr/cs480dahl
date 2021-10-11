#ifndef CAMERA_H
#define CAMERA_H

#include "object.h"
#include "graphics_headers.h"
#include <SDL2/SDL.h>

// Class representing a camera
class Camera {
public:
	Camera();
	~Camera();
	bool Initialize(int w, int h);
	void Update(unsigned int dt);
	glm::mat4 GetProjection();
	glm::mat4 GetView();

	// The camera updates some matricies when the window is resized.
	static int windowResizeEventListener(void* data, SDL_Event* event);

	virtual void Keyboard(const SDL_KeyboardEvent& e);
	void MouseButton(const SDL_MouseButtonEvent& e);
	void MouseMotion(const SDL_MouseMotionEvent& e);
	void MouseWheel(const SDL_MouseWheelEvent& e);

private:
	glm::mat4 projection;
	glm::mat4 view;

	glm::vec2 mouseLastPos;
	glm::vec2 cameraOrbitAngles;

	glm::vec3 posInSphere;
	glm::vec3 eyePos;
	glm::vec3 focusPos;
	Object* focusObject;
	float distanceFromFocusPos;
	float zoomScale;
	float minCap;

	static constexpr float orbitAngleChangeRate = 270.0;
};

#endif /* CAMERA_H */