#include "camera.h"
#include "object.h"

#include "application.h"
#include <algorithm> // for clamp

Camera::Camera(Application* engine) : app(engine) { }

Camera::~Camera() {
	// Unregister us as a listener to resize events
	SDL_DelEventWatch(windowResizeEventListener, this);
}

bool Camera::initialize(int w, int h) {
	// Init the starting eye and focus pos details
	eyePos = glm::vec3(0.0, 25, -25.0);
	focusPos = glm::vec3(8.0, 20.0, 8.0);

	// Determine the distance, starting pos in sphere and camera orbit angles based on eyePos and focusPos
	glm::vec3 diffVec = eyePos - focusPos;
	distanceFromFocusPos = glm::length(diffVec);
	posInSphere = glm::normalize(diffVec);
	cameraOrbitAngles = glm::vec2(180.0, 45.0); // if start eye and focus change this will have to change

	// Set some value for movement scale when scrolling
	zoomScale = 0.25;

	// Set a minimum distance cap
	minCap = 10.0;
	maxCap = 40;

	//Init projection matrices
	projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
								 float(w)/float(h), //Aspect Ratio, so Circles stay Circular
								 0.01f, //Distance to the near plane, normally a small value like this
								 10000000.0f); //Distance to the far plane,

	// Register us as a listener to window resize events
	SDL_AddEventWatch(windowResizeEventListener, this);
	return true;
}

void Camera::update(float dt) {
	// Capture input
	glm::ivec2 mousePos = glm::ivec2();
	Uint32 mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);

	// Check if right mouse button held
	if ((mouseState & SDL_BUTTON_RMASK) != 0) {
		// Get xy as xDiff yDiff from last frame
		int diffx = mousePos.x - mouseLastPos.x,
		diffy = mousePos.y - mouseLastPos.y;

		// Calculate xDiff and yDiff as a percentage of window size
		int w, h;
		SDL_GetWindowSize(SDL_GetWindowFromID(0), &w, &h);
		glm::vec2 movePercent;
		movePercent.x = (float) diffx / (float) w;
		movePercent.y = (float) diffy / (float) h;

		// Determine new angle based on percentage movement
		cameraOrbitAngles.x = -(movePercent.x * orbitAngleChangeRate) + cameraOrbitAngles.x;
		cameraOrbitAngles.y = -(movePercent.y * orbitAngleChangeRate) + cameraOrbitAngles.y;

		// Clamp y angles so 0 > y > 180 otherwise up normal flips
		cameraOrbitAngles.y = (cameraOrbitAngles.y < 0.0) ? 0.001 : cameraOrbitAngles.y;
		cameraOrbitAngles.y = (cameraOrbitAngles.y > 180.0) ? 179.999 : cameraOrbitAngles.y;

		// Set the pos in unit sphere to new angles within unit sphere
		posInSphere.x = glm::sin(glm::radians(cameraOrbitAngles.x)) * glm::sin(glm::radians(cameraOrbitAngles.y));
		posInSphere.z = glm::cos(glm::radians(cameraOrbitAngles.x)) * glm::sin(glm::radians(cameraOrbitAngles.y));
		posInSphere.y = glm::cos(glm::radians(cameraOrbitAngles.y));
	}

	// Store mouse position for next frame
	mouseLastPos = glm::vec2(mousePos.x, mousePos.y);

	// Get the focus position
	if (focusObj != nullptr)
		focusPos = focusObj->getPosition();

	// Get the eye position relative to the focus position and at some point in rotation sphere
	eyePos = focusPos + (posInSphere * distanceFromFocusPos);

	// Preform a raycast to ensure that the camera doesn't clip through
	auto result = app->getWorld()->raycast(focusPos, eyePos, CollisionGroups::CG_ENVIRONMENT);
	if(result) eyePos = result->point + result->normal * .1f;

	// View dynamically updates with camera control movements
	view = glm::lookAt(eyePos, focusPos, glm::vec3(0.0, 1.0, 0.0));
}

int Camera::windowResizeEventListener(void* data, SDL_Event* event) {
	// Check if this is a window resize event
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
		// Extract the camera from the event data
		Camera* camera = (Camera*) data;

		int w = event->window.data1, h = event->window.data2;
		camera->projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
											float(w)/float(h), //Aspect Ratio, so Circles stay Circular
											0.01f, //Distance to the near plane, normally a small value like this
											10000000.0f); //Distance to the far plane,

		// Resize the OpenGL viewport
		glViewport(0, 0, w, h);
	}

	return 0;
}

void Camera::keyboard(const SDL_KeyboardEvent& e) {
	if (e.type == SDL_KEYDOWN) {
		if (e.keysym.sym == SDLK_LSHIFT)
			// When holding shift decrease zoom amount
			zoomScale = 0.1;
	} else if (e.type == SDL_KEYUP) {
		if (e.keysym.sym == SDLK_LSHIFT) {
			// When shift is released increase zoom amount
			zoomScale = 0.25;
		}
	}
}

void Camera::mouseButton(const SDL_MouseButtonEvent& e) {
	// No purpose yet
}

void Camera::mouseMotion(const SDL_MouseMotionEvent& e) {
	// Check if right mouse button held
	if ((e.state & SDL_BUTTON_RMASK) != 0) {
		// Get xy as xDiff yDiff from last frame
		int diffx = e.x - mouseLastPos.x,
		diffy = e.y - mouseLastPos.y;

		// Calculate xDiff and yDiff as a percentage of window size
		int w, h;
		SDL_GetWindowSize(SDL_GetWindowFromID(e.windowID), &w, &h);
		glm::vec2 movePercent;
		movePercent.x = (float) diffx / (float) w;
		movePercent.y = (float) diffy / (float) h;

		// Determine new angle based on percentage movement
		cameraOrbitAngles.x = -(movePercent.x * orbitAngleChangeRate) + cameraOrbitAngles.x;
		cameraOrbitAngles.y = -(movePercent.y * orbitAngleChangeRate) + cameraOrbitAngles.y;

		// Clamp y angles so 0 > y > 180 otherwise up normal flips
		cameraOrbitAngles.y = (cameraOrbitAngles.y < 0.0) ? 0.001 : cameraOrbitAngles.y;
		cameraOrbitAngles.y = (cameraOrbitAngles.y > 180.0) ? 179.999 : cameraOrbitAngles.y;

		// Set the pos in unit sphere to new angles within unit sphere
		posInSphere.x = glm::sin(glm::radians(cameraOrbitAngles.x)) * glm::sin(glm::radians(cameraOrbitAngles.y));
		posInSphere.z = glm::cos(glm::radians(cameraOrbitAngles.x)) * glm::sin(glm::radians(cameraOrbitAngles.y));
		posInSphere.y = glm::cos(glm::radians(cameraOrbitAngles.y));
	}

	// Store mouse position for next frame
	mouseLastPos = glm::vec2(e.x, e.y);
}

void Camera::mouseWheel(const SDL_MouseWheelEvent& e) {
	// increase/decrease the distance from the focus pos based on wheel scroll up/down
	distanceFromFocusPos -= e.y * distanceFromFocusPos * zoomScale;

	// Limit the max zoom based on the scale of the focused celestial
	distanceFromFocusPos = std::min(maxCap, std::max(minCap, distanceFromFocusPos));
}
