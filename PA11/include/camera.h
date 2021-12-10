#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"
#include <SDL2/SDL.h>
#include <memory>

class Application;
class Object;

// Class representing a camera
class Camera {
public:
	Camera(Application* engine);
	~Camera();
	bool initialize(int w, int h);
	void update(float dt);
	glm::mat4 getProjection() { return projection; }
	glm::mat4 getView() { return view; }
	glm::vec3 getLookDirection() { return normalize(focusPos - eyePos);}
	void setFocus(std::shared_ptr<Object> object) {focusObj = object;}

	// The camera updates some matricies when the window is resized.
	static int windowResizeEventListener(void* data, SDL_Event* event);

	virtual void keyboard(const SDL_KeyboardEvent& e);
	void mouseButton(const SDL_MouseButtonEvent& e);
	void mouseMotion(const SDL_MouseMotionEvent& e);
	void mouseWheel(const SDL_MouseWheelEvent& e);

private:
	Application* app;
	std::shared_ptr<Object> focusObj = nullptr;

	glm::mat4 projection;
	glm::mat4 view;

	glm::vec2 mouseLastPos;
	glm::vec2 cameraOrbitAngles;

	glm::vec3 posInSphere;
	glm::vec3 eyePos;
	glm::vec3 focusPos;

	float distanceFromFocusPos;
	float zoomScale;
	float minCap, maxCap;

	static constexpr float orbitAngleChangeRate = 270.0;
};

#endif /* CAMERA_H */
