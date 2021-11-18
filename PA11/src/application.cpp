#include "application.h"
#include "light.h"
#include "window.h"
#include "graphics.h"
#include "sound.h"
#include "physics.h"
#include "camera.h"
#include "threadTimer.h"


bool Application::initialize(const Arguments& args) {
	bool ret = Engine::initialize(args);

	return ret;
}

void Application::update(float dt) {

}

void Application::drawGUI(){
	
}