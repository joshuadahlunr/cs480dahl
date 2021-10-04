#include "camera.h"

Camera::Camera() { }

Camera::~Camera() {
  // Unregister us as a listener to resize events
  SDL_DelEventWatch(windowResizeEventListener, this);
}

bool Camera::Initialize(int w, int h) {
  //--Init the view and projection matrices
  //  if you will be having a moving camera the view matrix will need to more dynamic
  //  ...Like you should update it before you render more dynamic
  //  for this project having them static will be fine
  view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
					  glm::vec3(0.0, 0.0, 0.0), //Focus point
					  glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

  projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
								 float(w)/float(h), //Aspect Ratio, so Circles stay Circular
								 0.01f, //Distance to the near plane, normally a small value like this
								 100.0f); //Distance to the far plane,

  // Register us as a listener to resize events
  SDL_AddEventWatch(windowResizeEventListener, this);
  return true;
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
								 100.0f); //Distance to the far plane,
    
    // Resize the OpenGL viewport
    glViewport(0, 0, w, h);
  }
  
  return 0;
}

glm::mat4 Camera::GetProjection() { return projection; }

glm::mat4 Camera::GetView() { return view; }
